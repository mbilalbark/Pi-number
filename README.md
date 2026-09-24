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

## Lisans

[LICENSE](LICENSE) dosyasına bakınız.
