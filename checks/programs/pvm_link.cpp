/* Does SIGEL's own PVM code link and run against real PVM? -- PORTING.md
   Phase P, step P4.  Driven by pvm-check.sh, which owns the daemon.

   WHAT IS LINKED.  The Makefile names SIG_GPFitnessTrainer.o and
   SIG_GPPVMData.o on the link line, so the linker takes them whether or not
   anything references them.  Those two are every object in the built core
   that leaves a pvm_* symbol undefined -- seven each, pvm_recv shared, 13
   distinct.  If SIGEL and libpvm3.a disagree, the link fails.

   WHAT IS RUN, which is less.  This calls SIG_GPPVMData::sendQStringToPVM and
   ::getQStringFromPVM twice, once ASCII and once multi-byte -- SIGEL's own
   wire format, its own pvm_initsend,
   pvm_pkint, pvm_pkstr, pvm_send, pvm_recv, pvm_upkint, pvm_upkstr.  That is
   7 of the 13.  The other 6 are SIG_GPFitnessTrainer's -- pvm_addhosts,
   pvm_delhosts, pvm_kill, pvm_probe, pvm_spawn, pvm_upkdouble -- and they
   spawn and manage sigel_slave, which cannot be built until Phase C.  Those
   six are link-checked only.  Do not read a PASS as more than that.

   WHY IT RUNS AT ALL, rather than only linking.  libasan.so exports weak
   xdr_double, xdr_int, xdrmem_create and the rest as interceptors, so under
   the sanitizers PVM's XDR references bind to those and the link succeeds
   with no -ltirpc and nothing behind them.  Running catches it: without
   -ltirpc the first PVM call dies with SEGV at pc 0 inside enc_xdr_init.

   The three constructor arguments are default-constructed.  SIG_Environment's
   default constructor loads terrain through vendored DynaMechs and leaks
   20,400 bytes in 51 allocations -- pre-existing, nothing to do with PVM, and
   why pvm-check.sh suppresses that one leak by function name rather than
   turning leak detection off. */

#include <cstdio>
#include <pvm3.h>

#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"
#include "SIGEL_GP/SIG_GPPVMData.h"

#define TAG	1

int main()
{
	int tid = pvm_mytid();
	if (tid < 0) {
		std::printf("pvm_mytid      FAILED, pvm returned %d\n", tid);
		std::printf("FAIL\n");
		return 1;
	}
	std::printf("tid            %#x\n", tid);

	SIGEL_Robot::SIG_Robot robot;
	SIGEL_Environment::SIG_Environment environment;
	SIGEL_Simulation::SIG_SimulationParameters parameters;
	SIGEL_GP::SIG_GPPVMData data(robot, environment, parameters,
			QString("simple"), false);

	std::printf("fitness name   \"%s\"\n",
			data.getFitnessFunctionName().toUtf8().constData());

	/* Two strings.  The second is multi-byte on purpose: sendQStringToPVM
	   used to size the receiver's buffer by character count while sending
	   UTF-8 bytes, so 200 u-umlauts wrote 401 bytes into 201 and ASan called
	   it a heap-buffer-overflow.  Fixed 2026-08-28; this is the regression
	   test, and it only bites under the sanitizers. */
	const QString ascii("sigel gp individual 0 fitness 0.0");
	const QString utf8 = QString(200, QChar(0x00FC)) + QString(50, QChar(0x20AC));

	bool ok = true;
	const struct { const char *what; const QString &s; } cases[] = {
		{ "ascii", ascii }, { "utf-8", utf8 },
	};
	for (const auto &c : cases) {
		data.sendQStringToPVM(c.s, tid, TAG);
		const QString got = data.getQStringFromPVM(tid, TAG);
		const bool same = (got == c.s);
		std::printf("%-14s %lld chars / %lld bytes  %s\n", c.what,
				(long long)c.s.length(), (long long)c.s.toUtf8().size(),
				same ? "exact" : "DIFFERS");
		ok = ok && same;
	}

	pvm_exit();
	std::printf("%s\n", ok ? "PASS" : "FAIL");
	return !ok;
}
