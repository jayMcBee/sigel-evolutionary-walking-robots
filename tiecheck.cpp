/*
  Does Qt 6 print a double the way SIGEL 1.3 did?  PORTING.md section 0.

    QTINC=$(qmake6 -query QT_INSTALL_HEADERS); QTLIB=$(qmake6 -query QT_INSTALL_LIBS)
    g++ -O2 -fPIC -o /tmp/tiecheck tiecheck.cpp -I"$QTINC" -I"$QTINC/QtCore" \
        -L"$QTLIB" -lQt6Core && /tmp/tiecheck

  NOT built by the Makefile and not a gate: nothing depends on the answer. It
  is here so the number can be re-derived rather than quoted.

  WHY %.6g IS THE 1.3 SIDE. 1.3's QTextStream does not format doubles at all.
  The oracle disassembled __ls__11QTextStreamd in libqt-mt.so.2.3.1: it
  assembles a format string byte by byte -- '%', '.', a precision clamped to
  99, then one of f e E g G with g the default -- and calls sprintf@plt. Every
  rounding decision is glibc's. It then reproduced all 9,547 numeric tokens of
  a real 120-frame POV-Ray sample with sprintf("%.6g", v), zero mismatches.

  MEASURED 2026-09-07: identical everywhere except exact dyadic halves, where
  Qt 6 rounds AWAY FROM ZERO and glibc rounds TO EVEN (0.5078125 -> 0.507813
  against 0.507812), and negative zero, which Qt 6 prints as "0". Only a dyadic
  rational can be an exact decimal tie; a physics simulation does not produce
  one, and the real sample above contains none. So the divergence is real and
  cannot reach any stream SIGEL writes.

  A WARNING ABOUT THE FIRST EIGHT VALUES BELOW. They are the oracle's published
  reference set, read out of a live glibc 2.2.5 process under gdb, and they do
  NOT discriminate: 999999.5 and 1048575.5 are true ties but round up under
  either rule. A probe built only from them reports "no difference" and is
  wrong. The dyadic halves whose preceding digit is EVEN are the ones that
  separate the two behaviours.
*/
#include <QString>
#include <QTextStream>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cmath>
#include <random>

static QString qt6(double v)
{
    QString s;
    QTextStream ts(&s);
    ts << v;                       // the default, which is what the writers use
    return s;
}
static QString libc(double v)
{
    char buf[64];
    std::snprintf(buf, sizeof buf, "%.6g", v);
    return QString::fromLatin1(buf);
}

static int mismatches = 0, checked = 0;
static void one(double v, const char *why)
{
    const QString a = qt6(v), b = libc(v);
    ++checked;
    if (a != b) {
        ++mismatches;
        std::printf("  DIFFER  %-22s qt6=[%s]  %%.6g=[%s]   %s\n",
                    QString::number(v, 'g', 17).toLatin1().constData(),
                    a.toLatin1().constData(), b.toLatin1().constData(), why);
    }
}

int main()
{
    std::printf("== the oracle's eight, measured in a live glibc 2.2.5 process ==\n");
    const double eight[] = { 999999.5, 1048575.5, 0.1234565, 1.0000005,
                             0.000123455, 2.5, 0.1000005, 0.5 };
    for (double v : eight) one(v, "oracle sample");

    std::printf("== dyadic halves: the ONLY values that can be exact ties ==\n");
    for (int e = -20; e <= 40; ++e) {
        const double base = std::ldexp(1.0, e);
        one(base + 0.5, "2^e + 0.5");
        one(base - 0.5, "2^e - 0.5");
        one(base * 1.5, "1.5 * 2^e");
    }
    for (double v = 0.5; v < 2.0e7; v += 1.0) one(v, "n + 0.5");

    std::printf("== negative zero, and the sign it used to carry ==\n");
    one(-0.0, "negative zero");
    one(0.0, "positive zero");

    std::printf("== random bit patterns, finite only ==\n");
    std::mt19937_64 rng(12345);
    for (int i = 0; i < 400000; ++i) {
        uint64_t bits = rng();
        double v;
        std::memcpy(&v, &bits, sizeof v);
        if (!std::isfinite(v)) continue;
        one(v, "random");
    }

    std::printf("\nchecked %d values, %d differ\n", checked, mismatches);
    return mismatches ? 1 : 0;
}
