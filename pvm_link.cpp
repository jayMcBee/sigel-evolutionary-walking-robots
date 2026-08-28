/* Does SIGEL's own PVM code link and run against real PVM? -- PORTING.md
   Phase P, step P4.  Driven by pvm-check.sh, which owns the daemon.

   The Makefile links this with SIG_GPFitnessTrainer.o and SIG_GPPVMData.o
   named on the command line, so both are pulled in whether or not anything
   here calls them.  Between them they are every object in the built core that
   leaves a pvm_* symbol undefined -- 13 of them, seven each, pvm_recv shared.
   If the library and SIGEL disagree, the link fails.

   WHY IT ALSO RUNS.  A link alone is not enough here.  libasan.so exports weak
   xdr_double, xdr_int, xdrmem_create and the rest as interceptors, so under
   the sanitizers PVM's XDR references resolve against those and the link
   succeeds with no -ltirpc and no real implementation behind them.  glibc
   keeps the same names as compat symbols (xdr_double@GLIBC_2.17), which a
   plain link will not bind to.  So the honest check is a round trip: pack,
   send, receive, unpack, compare.

   The values are the three types SIG_GPPVMData actually moves. */

#include <cstdio>
#include <cstring>
#include <sys/time.h>
#include <pvm3.h>

#define RECV_TIMEOUT_S	10

static int fail = 0;

static int check(const char *what, int rc)
{
	if (rc < 0) {
		std::printf("%-14s FAILED, pvm returned %d\n", what, rc);
		fail = 1;
	}
	return rc;
}

int main()
{
	struct timeval tmo = { RECV_TIMEOUT_S, 0 };
	double d_in = 9876.54321098765, d_out = 0;
	int    i_in = 31337,             i_out = 0;
	char   s_in[] = "sigel-core", s_out[32] = { 0 };
	int    tid, rc;

	tid = pvm_mytid();
	if (tid < 0) {
		std::printf("pvm_mytid      FAILED, pvm returned %d\n", tid);
		std::printf("FAIL\n");
		return 1;
	}
	std::printf("tid            %#x\n", tid);

	check("pvm_initsend",  pvm_initsend(PvmDataDefault));
	check("pvm_pkdouble",  pvm_pkdouble(&d_in, 1, 1));
	check("pvm_pkint",     pvm_pkint(&i_in, 1, 1));
	check("pvm_pkstr",     pvm_pkstr(s_in));
	check("pvm_send",      pvm_send(tid, 1));

	rc = check("pvm_trecv", pvm_trecv(tid, 1, &tmo));
	if (rc == 0) {
		std::printf("pvm_trecv      FAILED, nothing arrived in %d s\n",
				RECV_TIMEOUT_S);
		fail = 1;
	}

	if (!fail) {
		check("pvm_upkdouble", pvm_upkdouble(&d_out, 1, 1));
		check("pvm_upkint",    pvm_upkint(&i_out, 1, 1));
		check("pvm_upkstr",    pvm_upkstr(s_out));

		std::printf("double         %.17g %s\n", d_out,
				d_out == d_in ? "exact" : "DIFFERS");
		std::printf("int            %d %s\n", i_out,
				i_out == i_in ? "exact" : "DIFFERS");
		std::printf("string         \"%s\" %s\n", s_out,
				std::strcmp(s_out, s_in) ? "DIFFERS" : "exact");

		if (d_out != d_in || i_out != i_in || std::strcmp(s_out, s_in))
			fail = 1;
	}

	pvm_exit();
	std::printf("%s\n", fail ? "FAIL" : "PASS");
	return fail;
}
