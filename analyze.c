/*
 * analyze.c - Pi sayisinin ondalik basamaklari icin hizli analiz araci.
 *
 * Kullanim:
 *   analyze extract <girdi.txt> <cikti.bin>       -> sadece rakamlari cikarir
 *   analyze freq    <dosya>                        -> 0-9 rakam frekanslari
 *   analyze find    <dosya> <oruntu> [--all]        -> oruntu + tersini arar
 *   analyze kmer    <dosya> <k> [topN]              -> tum k-hane oruntularin frekansi
 *
 * <dosya> pi1.txt..pi5.txt gibi duz rakam dokumleri, pi6.txt gibi QPI
 * ciktisi (basliklar ve " : <konum>" sayaclari icerir) ya da "extract"
 * ile uretilmis saf rakam dosyasi olabilir; format otomatik algilanir.
 *
 * Derleme:
 *   gcc -O2 -o analyze analyze.c
 *   veya (MSVC)  cl /O2 analyze.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------------------------------------------------- */
/* Dosyayi tamamen belleğe okur.                                     */
/* ---------------------------------------------------------------- */
static unsigned char *read_file(const char *path, long long *out_size) {
    FILE *fp = fopen(path, "rb");
    long size;
    unsigned char *buf;
    size_t got;

    if (!fp) {
        fprintf(stderr, "Dosya acilamadi: %s\n", path);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (size < 0) {
        fprintf(stderr, "Dosya boyutu okunamadi: %s\n", path);
        fclose(fp);
        return NULL;
    }

    buf = (unsigned char *)malloc((size_t)size + 1);
    if (!buf) {
        fprintf(stderr, "Bellek ayrilamadi (%ld bayt icin)\n", size);
        fclose(fp);
        return NULL;
    }
    got = fread(buf, 1, (size_t)size, fp);
    fclose(fp);
    buf[got] = '\0';
    *out_size = (long long)got;
    return buf;
}

/* Not: ftell() bazi platformlarda 32-bit 'long' dondurur (~2GB sinir).
 * pi6.txt (~136MB) ve makul buyuklukteki gelecekteki dosyalar icin
 * yeterlidir; 2GB'i asan dosyalar icin platforma ozgu 64-bit fseek/ftell
 * (orn. _fseeki64/_ftelli64 ya da fseeko/ftello) kullanilmalidir. */

/* ---------------------------------------------------------------- */
/* Ham metinden sadece pi'nin ondalik basamaklarini cikarir.         */
/*                                                                    */
/* Iki format destekleniyor:                                         */
/*  1) Duz dokum (pi1..pi5.txt): sadece rakam ve bosluk/satir sonu.   */
/*     Butun rakamlar dogrudan alinir.                                */
/*  2) QPI ciktisi (pi6.txt): "Pi = 3." isaretinden sonra, her         */
/*     satirda rakam gruplari ve sonunda " : <toplam basamak>"        */
/*     sayaci bulunur. Sayac kismi pi basamagi OLMADIGI icin ':'       */
/*     karakterinden itibaren satir sonuna kadar atlanir.             */
/* ---------------------------------------------------------------- */
static unsigned char *extract_digits(const unsigned char *buf, long long size, long long *out_len) {
    static const char marker[] = "Pi = 3.";
    const long long marker_len = (long long)(sizeof(marker) - 1);
    unsigned char *out;
    long long out_len_local = 0;
    long long mpos = -1;
    long long i;

    for (i = 0; i + marker_len <= size; i++) {
        if (memcmp(buf + i, marker, (size_t)marker_len) == 0) {
            mpos = i;
            break;
        }
    }

    out = (unsigned char *)malloc((size_t)size + 1);
    if (!out) {
        fprintf(stderr, "Bellek ayrilamadi\n");
        return NULL;
    }

    if (mpos >= 0) {
        long long pos = mpos + marker_len;
        while (pos < size) {
            long long line_start = pos;
            long long line_end = pos;
            long long colon = -1;
            long long k;

            while (line_end < size && buf[line_end] != '\n' && buf[line_end] != '\r')
                line_end++;

            for (k = line_start; k < line_end; k++) {
                if (buf[k] == ':') { colon = k; break; }
            }
            {
                long long copy_end = (colon >= 0) ? colon : line_end;
                for (k = line_start; k < copy_end; k++) {
                    if (buf[k] >= '0' && buf[k] <= '9')
                        out[out_len_local++] = buf[k];
                }
            }
            pos = line_end;
            while (pos < size && (buf[pos] == '\n' || buf[pos] == '\r'))
                pos++;
        }
    } else {
        for (i = 0; i < size; i++) {
            if (buf[i] >= '0' && buf[i] <= '9')
                out[out_len_local++] = buf[i];
        }
    }

    out[out_len_local] = '\0';
    *out_len = out_len_local;
    return out;
}

/* Bir dosya yolundan (ham veya zaten saf rakam) rakam dizisini uretir. */
static unsigned char *load_digits(const char *path, long long *out_len) {
    long long raw_size;
    unsigned char *raw = read_file(path, &raw_size);
    unsigned char *digits;

    if (!raw) return NULL;
    digits = extract_digits(raw, raw_size, out_len);
    free(raw);
    return digits;
}

/* ---------------------------------------------------------------- */
/* extract komutu                                                    */
/* ---------------------------------------------------------------- */
static int cmd_extract(const char *in_path, const char *out_path) {
    long long len;
    unsigned char *digits = load_digits(in_path, &len);
    FILE *out;

    if (!digits) return 1;

    out = fopen(out_path, "wb");
    if (!out) {
        fprintf(stderr, "Cikti dosyasi acilamadi: %s\n", out_path);
        free(digits);
        return 1;
    }
    fwrite(digits, 1, (size_t)len, out);
    fclose(out);
    free(digits);

    printf("%lld basamak cikarildi -> %s\n", len, out_path);
    return 0;
}

/* ---------------------------------------------------------------- */
/* freq komutu                                                       */
/* ---------------------------------------------------------------- */
static int cmd_freq(const char *path) {
    long long len, i;
    unsigned char *digits = load_digits(path, &len);
    long long count[10] = {0};
    double expected;

    if (!digits) return 1;

    for (i = 0; i < len; i++)
        count[digits[i] - '0']++;

    printf("Toplam basamak: %lld\n\n", len);
    expected = (double)len / 10.0;
    for (i = 0; i < 10; i++) {
        double pct = len ? (100.0 * (double)count[i] / (double)len) : 0.0;
        double dev = len ? (100.0 * ((double)count[i] - expected) / expected) : 0.0;
        printf("  %lld : %10lld  (%6.3f%%, beklenenden %+.3f%%)\n", i, count[i], pct, dev);
    }

    free(digits);
    return 0;
}

/* ---------------------------------------------------------------- */
/* KMP alt-dizi arama: O(n + m), oruntu uzunlugundan bagimsiz hizli.  */
/* ---------------------------------------------------------------- */
static long long kmp_search(const unsigned char *text, long long n,
                             const unsigned char *pat, long long m,
                             long long *positions, long long max_positions) {
    long long *lps;
    long long count = 0;
    long long len = 0, i, j;

    if (m == 0 || n < m) return 0;

    lps = (long long *)malloc(sizeof(long long) * (size_t)m);
    lps[0] = 0;
    i = 1;
    while (i < m) {
        if (pat[i] == pat[len]) {
            len++;
            lps[i] = len;
            i++;
        } else if (len != 0) {
            len = lps[len - 1];
        } else {
            lps[i] = 0;
            i++;
        }
    }

    j = 0;
    for (i = 0; i < n; i++) {
        while (j > 0 && text[i] != pat[j])
            j = lps[j - 1];
        if (text[i] == pat[j])
            j++;
        if (j == m) {
            if (count < max_positions)
                positions[count] = i - m + 1;
            count++;
            j = lps[j - 1];
        }
    }

    free(lps);
    return count;
}

/* Once sayim gecisi (positions=NULL), sonra sadece gosterilecek kadar
 * pozisyonu dolduran ikinci gecis: buyuk dosyalarda --all ile bile
 * asiri bellek ayirmayi engeller. */
static void find_and_report(const char *label, const unsigned char *text, long long n,
                             const unsigned char *pat, long long m, int show_all) {
    long long count = kmp_search(text, n, pat, m, NULL, 0);
    long long shown = show_all ? count : (count < 20 ? count : 20);
    long long *positions = NULL;
    long long k;
    double expected;

    if (shown > 0) {
        positions = (long long *)malloc(sizeof(long long) * (size_t)shown);
        kmp_search(text, n, pat, m, positions, shown);
    }

    printf("%s \"%.*s\" : %lld kez bulundu\n", label, (int)m, pat, count);
    if (shown > 0) {
        printf("  pozisyonlar (0-indeksli): ");
        for (k = 0; k < shown; k++)
            printf("%lld%s", positions[k], (k + 1 < shown) ? ", " : "");
        if (count > shown) printf(", ... (+%lld tane daha, hepsini gormek icin --all)", count - shown);
        printf("\n");
    }

    expected = (double)(n - m + 1);
    {
        long long p;
        double denom = 1.0;
        for (p = 0; p < m; p++) denom *= 10.0;
        expected /= denom;
    }
    printf("  rastgele dagilimda beklenen sayi: ~%.4f\n\n", expected);

    free(positions);
}

/* ---------------------------------------------------------------- */
/* find komutu: verilen oruntuyu ve tersini arar.                    */
/* ---------------------------------------------------------------- */
static int cmd_find(const char *path, const char *pattern, int show_all) {
    long long len, m, i;
    unsigned char *digits = load_digits(path, &len);
    unsigned char *rev;

    if (!digits) return 1;

    m = (long long)strlen(pattern);
    for (i = 0; i < m; i++) {
        if (pattern[i] < '0' || pattern[i] > '9') {
            fprintf(stderr, "Oruntu sadece rakamlardan olusmali: %s\n", pattern);
            free(digits);
            return 1;
        }
    }
    if (m == 0 || m > len) {
        fprintf(stderr, "Gecersiz oruntu uzunlugu.\n");
        free(digits);
        return 1;
    }

    find_and_report("Oruntu", digits, len, (const unsigned char *)pattern, m, show_all);

    rev = (unsigned char *)malloc((size_t)m + 1);
    for (i = 0; i < m; i++) rev[i] = pattern[m - 1 - i];
    rev[m] = '\0';

    if (memcmp(rev, pattern, (size_t)m) != 0) {
        find_and_report("Ters oruntu", digits, len, rev, m, show_all);
    }

    free(rev);
    free(digits);
    return 0;
}

/* ---------------------------------------------------------------- */
/* kmer komutu: uzunlugu k olan TUM oruntulerin frekans tablosu.     */
/* Kaydirmali (rolling) taban-10 hash ile O(n) sayim.                 */
/* ---------------------------------------------------------------- */
typedef struct { long long value; long long count; } kmer_entry;

static int cmp_kmer_desc(const void *a, const void *b) {
    long long ca = ((const kmer_entry *)a)->count;
    long long cb = ((const kmer_entry *)b)->count;
    if (ca < cb) return 1;
    if (ca > cb) return -1;
    return 0;
}

static void print_kmer_value(long long value, int k) {
    char buf[32];
    int i;
    for (i = k - 1; i >= 0; i--) {
        buf[i] = (char)('0' + (value % 10));
        value /= 10;
    }
    buf[k] = '\0';
    printf("%s", buf);
}

static int cmd_kmer(const char *path, int k, long long top_n) {
    long long len, i;
    unsigned char *digits = load_digits(path, &len);
    long long size = 1;
    long long *freq;
    long long pow10k_1 = 1;
    long long val = 0;
    long long windows;
    kmer_entry *entries;
    double expected;

    if (!digits) return 1;

    if (k <= 0 || k > 7) {
        fprintf(stderr, "k, 1 ile 7 arasinda olmali (10^k tablo bellekte tutulur).\n"
                         "Daha uzun tek bir oruntu icin 'find' komutunu kullanin.\n");
        free(digits);
        return 1;
    }
    if (len < k) {
        fprintf(stderr, "Rakam sayisi k'dan kucuk.\n");
        free(digits);
        return 1;
    }

    for (i = 0; i < k; i++) size *= 10;
    for (i = 0; i < k - 1; i++) pow10k_1 *= 10;

    freq = (long long *)calloc((size_t)size, sizeof(long long));
    if (!freq) {
        fprintf(stderr, "Bellek ayrilamadi.\n");
        free(digits);
        return 1;
    }

    for (i = 0; i < k; i++) val = val * 10 + (digits[i] - '0');
    freq[val]++;
    for (i = k; i < len; i++) {
        int lead = digits[i - k] - '0';
        int cur = digits[i] - '0';
        val = (val - (long long)lead * pow10k_1) * 10 + cur;
        freq[val]++;
    }

    windows = len - k + 1;
    expected = (double)windows / (double)size;

    printf("k=%d icin %lld farkli oruntu, toplam %lld pencere, beklenen ortalama %.3f\n\n",
           k, size, windows, expected);

    entries = (kmer_entry *)malloc(sizeof(kmer_entry) * (size_t)size);
    for (i = 0; i < size; i++) { entries[i].value = i; entries[i].count = freq[i]; }
    qsort(entries, (size_t)size, sizeof(kmer_entry), cmp_kmer_desc);

    if (top_n <= 0) top_n = 10;
    if (top_n > size) top_n = size;

    printf("En sik gorulen %lld oruntu:\n", top_n);
    for (i = 0; i < top_n; i++) {
        printf("  ");
        print_kmer_value(entries[i].value, k);
        printf(" : %lld kez (beklenenin %+.2f%%)\n", entries[i].count,
               expected > 0 ? 100.0 * (entries[i].count - expected) / expected : 0.0);
    }

    printf("\nEn az gorulen %lld oruntu:\n", top_n);
    for (i = 0; i < top_n; i++) {
        long long idx = size - 1 - i;
        printf("  ");
        print_kmer_value(entries[idx].value, k);
        printf(" : %lld kez (beklenenin %+.2f%%)\n", entries[idx].count,
               expected > 0 ? 100.0 * (entries[idx].count - expected) / expected : 0.0);
    }

    {
        long long zero_count = 0;
        for (i = 0; i < size; i++) if (entries[i].count == 0) zero_count++;
        printf("\nHic gorulmeyen oruntu sayisi: %lld / %lld\n", zero_count, size);
    }

    free(entries);
    free(freq);
    free(digits);
    return 0;
}

/* ---------------------------------------------------------------- */

static void print_usage(const char *prog) {
    fprintf(stderr,
        "Kullanim:\n"
        "  %s extract <girdi.txt> <cikti.bin>\n"
        "  %s freq    <dosya>\n"
        "  %s find    <dosya> <oruntu> [--all]\n"
        "  %s kmer    <dosya> <k> [topN]\n",
        prog, prog, prog, prog);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "extract") == 0 && argc == 4) {
        return cmd_extract(argv[2], argv[3]);
    } else if (strcmp(argv[1], "freq") == 0 && argc == 3) {
        return cmd_freq(argv[2]);
    } else if (strcmp(argv[1], "find") == 0 && (argc == 4 || argc == 5)) {
        int show_all = (argc == 5 && strcmp(argv[4], "--all") == 0);
        return cmd_find(argv[2], argv[3], show_all);
    } else if (strcmp(argv[1], "kmer") == 0 && (argc == 4 || argc == 5)) {
        int k = atoi(argv[3]);
        long long top_n = (argc == 5) ? (long long)atol(argv[4]) : 10;
        return cmd_kmer(argv[2], k, top_n);
    }

    print_usage(argv[0]);
    return 1;
}
