/* One PVM round trip -- PORTING.md Phase P, step P3.  Driven by pvm-check.sh,
   which owns the daemon.

   Joins the virtual machine, packs one of each type SIGEL's SIG_GPPVMData
   sends, sends it to itself, unpacks it, and compares.  That is what P4 needs
   to be true: libpvm3.a reaches a live pvmd3, and XDR does not move a number.
   A build that was never run is what the Phase P research got wrong, so this
   exists to stop that happening twice.

   The send is to our own tid, which still goes out to the daemon and back --
   PvmDataDefault XDR-encodes on pack and decodes on unpack either way.
   Measured with -Wl,--wrap=xdr_double,--wrap=xdr_int: 2 and 30 calls.

   pvm_trecv, not pvm_recv: a plain pvm_recv against a broken PVM blocks for
   ever, and a check that hangs is worse than one that fails. */

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <pvm3.h>

#define RECV_TIMEOUT_S	10

/* Every PVM call returns < 0 on failure.  Checking them is the difference
   between "the numbers matched" and "we never sent anything". */
static int fail = 0;

static int check(const char *what, int rc)
{
	if (rc < 0) {
		printf("%-14s FAILED, pvm returned %d\n", what, rc);
		fail = 1;
	}
	return rc;
}

int main(void)
{
	struct timeval tmo = { RECV_TIMEOUT_S, 0 };
	double d_in = 1234.56789012345, d_out = 0;
	int    i_in = -424242,           i_out = 0;
	char   s_in[] = "sigel", s_out[32] = { 0 };
	int    tid, rc;

	tid = pvm_mytid();
	if (tid < 0) {
		printf("pvm_mytid      FAILED, pvm returned %d\n", tid);
		printf("FAIL\n");
		return 1;
	}
	printf("tid            %#x\n", tid);

	check("pvm_initsend",  pvm_initsend(PvmDataDefault));
	check("pvm_pkdouble",  pvm_pkdouble(&d_in, 1, 1));
	check("pvm_pkint",     pvm_pkint(&i_in, 1, 1));
	check("pvm_pkstr",     pvm_pkstr(s_in));
	check("pvm_send",      pvm_send(tid, 1));

	rc = check("pvm_trecv", pvm_trecv(tid, 1, &tmo));
	if (rc == 0) {						/* 0 means the timeout expired */
		printf("pvm_trecv      FAILED, nothing arrived in %d s\n",
				RECV_TIMEOUT_S);
		fail = 1;
	}

	if (!fail) {
		check("pvm_upkdouble", pvm_upkdouble(&d_out, 1, 1));
		check("pvm_upkint",    pvm_upkint(&i_out, 1, 1));
		check("pvm_upkstr",    pvm_upkstr(s_out));

		printf("double         %.17g %s\n", d_out,
				d_out == d_in ? "exact" : "DIFFERS");
		printf("int            %d %s\n", i_out,
				i_out == i_in ? "exact" : "DIFFERS");
		printf("string         \"%s\" %s\n", s_out,
				strcmp(s_out, s_in) ? "DIFFERS" : "exact");

		if (d_out != d_in || i_out != i_in || strcmp(s_out, s_in))
			fail = 1;
	}

	pvm_exit();
	printf("%s\n", fail ? "FAIL" : "PASS");
	return fail;
}
