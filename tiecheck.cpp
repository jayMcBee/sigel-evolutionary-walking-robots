/*
  Does Qt 6 print a double the way SIGEL 1.3 did?  PORTING.md section 0 and C5.

    QTINC=$(qmake6 -query QT_INSTALL_HEADERS); QTLIB=$(qmake6 -query QT_INSTALL_LIBS)
    g++ -O2 -fPIC -o /tmp/tiecheck tiecheck.cpp -I"$QTINC" -I"$QTINC/QtCore" \
        -L"$QTLIB" -lQt6Core && /tmp/tiecheck

  NOT built by the Makefile and NOT a gate. It exists so the numbers can be
  re-derived rather than quoted. Exit status is deliberately 0: differences are
  the expected result, not a failure.

  WHY %.*g IS THE 1.3 SIDE. 1.3's QTextStream does not format doubles itself --
  it builds a format string and calls sprintf (qtextstream.cpp:1776-1805 in the
  vendored source; the oracle independently disassembled __ls__11QTextStreamd in
  libqt-mt.so.2.3.1 and found the same). So every rounding decision is glibc's.

  THE PRECISIONS ARE THE POINT. SIGEL does not write at one precision:

    SIG_Renderer.cpp, vectorToPovray     setRealNumberPrecision( 5 )   every POV <x,y,z>
    default                                        ( 6 )   .exp, .rrb, POV matrix
    SIG_GPPVMData.cpp:116,157                      ( 50 )  master<->slave transfer

  An earlier version of this probe tested ONLY the default 6 and concluded the
  difference could not reach any stream. That was wrong twice over: it measured
  a precision two of the three writers do not use, and it argued from "only a
  dyadic rational can be an exact tie" -- which excludes nothing, because EVERY
  finite double is a dyadic rational. The real condition is that the exact
  decimal expansion runs one digit past the print precision and ends in 5.
  Found by review.
*/
#include <QString>
#include <QTextStream>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cmath>
#include <random>
#include <functional>

static QString qt6(double v, int prec)
{
    QString s;
    QTextStream ts(&s);
    ts.setRealNumberPrecision(prec);
    ts << v;
    return s;
}
static QString libc(double v, int prec)
{
    char buf[512];
    std::snprintf(buf, sizeof buf, "%.*g", prec, v);
    return QString::fromLatin1(buf);
}

struct Count { long long checked = 0, differ = 0; };

static void run(const char *what, int prec, Count &c,
                const std::function<void(const std::function<void(double)> &)> &gen)
{
    int shown = 0;
    gen([&](double v) {
        const QString a = qt6(v, prec), b = libc(v, prec);
        ++c.checked;
        if (a != b) {
            ++c.differ;
            if (shown < 2) { ++shown;
                std::printf("      e.g. %-24s qt6=[%s]  %%.%dg=[%s]\n",
                            QString::number(v, 'g', 17).toLatin1().constData(),
                            a.toLatin1().constData(), prec,
                            b.toLatin1().constData()); }
        }
    });
    std::printf("  %-38s prec %2d  %10lld checked  %9lld differ  %5.2f%%\n",
                what, prec, c.checked, c.differ,
                c.checked ? 100.0 * double(c.differ) / double(c.checked) : 0.0);
}

int main()
{
    // Exact binary fractions -- "exactly what geometry is made of", C5's words.
    auto sixteenths = [](const std::function<void(double)> &f) {
        for (int i = 0; i <= 1600; ++i) f(i / 16.0);
    };
    auto sixtyfourths = [](const std::function<void(double)> &f) {
        for (int i = 0; i <= 6400; ++i) f(i / 64.0);
    };
    auto twofiftysixths = [](const std::function<void(double)> &f) {
        for (int i = 0; i <= 2560; ++i) f(i / 256.0);
    };
    // Values shaped like a physics step: ordinary doubles in SIGEL's range.
    auto physicsRange = [](const std::function<void(double)> &f) {
        std::mt19937_64 rng(20260907);
        std::uniform_real_distribution<double> d(-1000.0, 1000.0);
        for (int i = 0; i < 300000; ++i) f(d(rng));
    };
    // Uniform over bit patterns. Kept only to show what it CANNOT see.
    auto bitPatterns = [](const std::function<void(double)> &f) {
        std::mt19937_64 rng(12345);
        for (int i = 0; i < 300000; ++i) {
            uint64_t bits = rng(); double v;
            std::memcpy(&v, &bits, sizeof v);
            if (std::isfinite(v)) f(v);
        }
    };

    for (int prec : {5, 6, 50}) {
        std::printf("== precision %d ==\n", prec);
        Count a, b, c, d, e;
        run("multiples of 1/16 over [0,100]",   prec, a, sixteenths);
        run("multiples of 1/64 over [0,100]",   prec, b, sixtyfourths);
        run("multiples of 1/256 over [0,10]",   prec, c, twofiftysixths);
        run("ordinary doubles in [-1000,1000]", prec, d, physicsRange);
        run("uniform bit patterns",             prec, e, bitPatterns);
        std::printf("\n");
    }

    std::printf("== signed zero, at each precision ==\n");
    for (int prec : {5, 6, 50})
        std::printf("  -0.0  prec %2d   qt6=[%s]  %%.%dg=[%s]\n", prec,
                    qt6(-0.0, prec).toLatin1().constData(), prec,
                    libc(-0.0, prec).toLatin1().constData());

    std::printf("\n== negatives and large values, which the old probe never tried ==\n");
    for (double v : { -100000.5, 1234565000000.0, 1000005000000.0, 0.703125 })
        for (int prec : {5, 6})
            if (qt6(v, prec) != libc(v, prec))
                std::printf("  %-18s prec %2d  qt6=[%s]  %%.%dg=[%s]\n",
                            QString::number(v, 'g', 17).toLatin1().constData(), prec,
                            qt6(v, prec).toLatin1().constData(), prec,
                            libc(v, prec).toLatin1().constData());
    return 0;
}
