# Pi-number

Pi sayısının ondalık basamakları üzerinde çalışan küçük bir C projesi: basamak frekansı sayımı, belirli bir örüntünün (dizinin) aranması ve istatistiksel örüntü taraması.

## Basamak dosyaları

| Dosya | Basamak sayısı | Format |
|---|---|---|
| [pi1.txt](pi1.txt) | 1.000 | düz rakam dökümü |
| [pi2.txt](pi2.txt) | 9.999 | düz rakam dökümü |
| [pi3.txt](pi3.txt) | 99.998 | düz rakam dökümü |
| [pi4.txt](pi4.txt) | 1.000.000 | düz rakam dökümü |
| [pi5.txt](pi5.txt) | 10.000.000 | düz rakam dökümü |
| pi6.txt | 100.000.000 | QPI çıktısı (başlık + her satırda `" : <konum sayacı>"` içerir) |

`pi1.txt`–`pi5.txt` dosyaları boşluklarla ayrılmış düz rakam dökümleridir. `pi6.txt`, [archive.org](https://archive.org/details/Pi_to_100000000_places) üzerindeki QPI-Quick Pi v2.90 çıktısıdır ve her satırın sonunda o ana kadarki toplam basamak sayısını gösteren bir `: <sayı>` sayacı bulunur — bu sayaç pi'nin basamağı **değildir**, analiz araçları bunu otomatik olarak ayırt edip atlar. Boyutu (136 MB) GitHub'ın dosya başına 100 MB sınırını aştığı için bu depoya eklenmedi; yukarıdaki linkten indirip yerel olarak kullanabilirsiniz.

## Araçlar

### [pi.c](pi.c)

İlk, basit versiyon. `pi1.txt`–`pi5.txt` formatındaki bir dosyada rakam frekansı sayar.

```powershell
gcc -o pi pi.c
./pi
```

### [analyze.c](analyze.c)

Daha kapsamlı ve hızlı analiz aracı. Hem düz format hem QPI (`pi6.txt` tarzı) formatı otomatik algılar, 100 milyon basamaklı dosyalarda bile tüm komutlar bir saniyenin altında çalışır.

**Derleme:**

```powershell
gcc -O2 -o analyze.exe analyze.c
```

Windows'ta gcc yoksa: [WinLibs MinGW-w64](https://winlibs.com/) veya MSYS2 kurup PATH'e ekleyin (`winget install BrechtSanders.WinLibs.POSIX.UCRT`), ya da MSVC ile `cl /O2 analyze.c`.

**Komutlar:**

```
analyze extract <girdi.txt> <cikti.bin>     rakamları saf haliyle çıkarır
analyze freq    <dosya>                      0-9 rakam frekansı
analyze find    <dosya> <örüntü> [--all]     örüntüyü ve tersini arar
analyze kmer    <dosya> <k> [topN]           tüm k-haneli örüntülerin frekans tablosu
```

`freq` ve `kmer`, `pi6.txt` gibi ham dosyaları doğrudan kabul eder; ayrıca `extract` ile önceden çıkarılmış saf rakam dosyaları üzerinde de çalışır (tekrarlanan analizlerde ayrıştırma adımını atlayıp daha da hızlı sonuç verir).

#### freq — rakam frekansı

```powershell
.\analyze.exe freq pi6.txt
```

```
Toplam basamak: 100000000

  0 :    9999922  (10.000%, beklenenden -0.001%)
  1 :   10002475  (10.002%, beklenenden +0.025%)
  ...
```

#### find — örüntü arama (KMP, O(n+m))

Belirtilen örüntüyü **ve tersini** arar; örüntü uzunluğundan bağımsız olarak hızlıdır.

```powershell
.\analyze.exe find pi6.txt 0123456789 --all
```

İlk 100 milyon basamakta `0123456789` dizisi (ve tersi `9876543210`) hiç geçmiyor.

#### kmer — tüm k-haneli örüntülerin taraması (rolling hash, O(n))

`k` haneli (1-7 arası) her olası örüntünün kaç kez geçtiğini tek geçişte sayar; en sık/en az görülenleri ve hiç görülmeyenleri listeler. Pi'nin basamaklarının istatistiksel olarak rastgele dağılıp dağılmadığını (belirgin bir örüntü olup olmadığını) görmek için kullanışlıdır.

```powershell
.\analyze.exe kmer pi6.txt 4 10
```

k, `10^k` büyüklüğünde bir tabloyu bellekte tuttuğu için 7 ile sınırlıdır (k=7 için ~240 MB RAM). Daha uzun **belirli** bir örüntü aramak için `find` kullanın — onun uzunluk sınırı yoktur.

## Performans

100.000.000 basamaklık `pi6.txt` üzerinde `freq`, `find` ve `kmer` komutlarının her biri 0.2–0.6 saniye içinde tamamlanıyor.

## Raporlar (pi6.txt, 100.000.000 basamak)

Dosyayı indirip kendiniz çalıştırmak istemiyorsanız diye `analyze.exe` ile üretilmiş gerçek sonuçlar aşağıda. Tüm komutlar 136 MB'lık `pi6.txt` üzerinde 0.2–0.6 saniyede tamamlandı.

### Rakam frekansı (`freq`)

Her rakam beklenen ~%10'a çok yakın; en büyük sapma %0.065 (rakam 5). Belirgin bir dengesizlik yok.

| Rakam | Adet | Yüzde | Beklenenden sapma |
|---|---|---|---|
| 0 | 9.999.922 | 10.000% | −0.001% |
| 1 | 10.002.475 | 10.002% | +0.025% |
| 2 | 10.001.092 | 10.001% | +0.011% |
| 3 | 9.998.442 | 9.998% | −0.016% |
| 4 | 10.003.863 | 10.004% | +0.039% |
| 5 | 9.993.478 | 9.993% | −0.065% |
| 6 | 9.999.417 | 9.999% | −0.006% |
| 7 | 9.999.610 | 10.000% | −0.004% |
| 8 | 10.002.180 | 10.002% | +0.022% |
| 9 | 9.999.521 | 10.000% | −0.005% |

### Belirli örüntü aramaları (`find`)

| Örüntü | Sonuç | Not |
|---|---|---|
| `0123456789` (ve tersi `9876543210`) | **0 kez** | ilk 100 milyon basamakta hiç geçmiyor |
| `123456789` (ve tersi `987654321`) | **0 kez** | bu da hiç geçmiyor |
| `14159` (ve tersi `95141`) | 979 / 1010 kez | beklenen ~1000, ilk konum 0 (pi'nin başındaki basamaklar) |
| `999999` | **107 kez**, ilk konum **761** | ünlü ["Feynman noktası"](https://en.wikipedia.org/wiki/Feynman_point) — pi'nin ilk 762. basamağından itibaren art arda 6 tane 9 gelir, bilinen bir matematik ilginçliği, buradan da doğrulandı |

### Tüm k-haneli örüntülerin taraması (`kmer`)

3, 4 ve 6 haneli tüm örüntüler tek tek tarandı; hiçbir uzunlukta **hiç görülmeyen örüntü yok** (0/1.000, 0/10.000, 0/1.000.000) — yani her kombinasyon en az bir kez geçmiş. Sapmalar örneklem büyüklüğüne göre istatistiksel olarak beklenen aralıkta, kalıcı bir "sıcak" ya da "soğuk" örüntü yok.

| k | Farklı örüntü | En sık | En az |
|---|---|---|---|
| 3 | 1.000 | `941` → 100.917 kez (+0.92%) | `478` → 99.006 kez (−0.99%) |
| 4 | 10.000 | `7017` → 10.416 kez (+4.16%) | `2280` → 9.618 kez (−3.82%) |
| 6 | 1.000.000 | `116972` → 154 kez (+54.00%) | `653189` → 56 kez (−44.00%) |

**Sonuç:** İlk 100 milyon basamakta pi'nin rakamları, beklenen istatistiksel rastgeleliğin dışına çıkan bir örüntü göstermiyor — dağılım "normal sayı" (normal number) varsayımıyla tutarlı.

## Lisans

[LICENSE](LICENSE) dosyasına bakınız.
