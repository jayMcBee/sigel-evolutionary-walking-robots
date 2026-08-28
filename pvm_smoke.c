/* One PVM round trip -- PORTING.md Phase P, step P3.  Driven by pvm-check.sh,
   which owns the daemon.

   Joins the virtual machine, packs one of each type SIGEL's SIG_GPPVMData
   sends, sends it to itself, unpacks it, and compares.  That is the whole of
   what P4 needs to be true: libpvm3.a reaches a live pvmd3, and XDR does not
   move a number.  A build that was never run is what the Phase P research got
   wrong, so this exists to stop that happening twice. */
#include <stdio.h>
#include <string.h>
#include <pvm3.h>
int main(void)
{
	int tid = pvm_mytid();
	if (tid < 0) { printf("pvm_mytid failed: %d\n", tid); return 1; }
	printf("tid      %#x\n", tid);

	double d_in = 1234.56789012345, d_out = 0;
	int    i_in = -424242,           i_out = 0;
	char   s_in[] = "sigel", s_out[32] = {0};

	pvm_initsend(PvmDataDefault);
	pvm_pkdouble(&d_in, 1, 1);
	pvm_pkint(&i_in, 1, 1);
	pvm_pkstr(s_in);
	pvm_send(tid, 1);                 /* to ourselves */
	pvm_recv(tid, 1);
	pvm_upkdouble(&d_out, 1, 1);
	pvm_upkint(&i_out, 1, 1);
	pvm_upkstr(s_out);

	int ok = (d_out == d_in) && (i_out == i_in) && !strcmp(s_out, s_in);
	printf("double   %.17g %s\n", d_out, d_out == d_in ? "exact" : "DIFFERS");
	printf("int      %d %s\n", i_out, i_out == i_in ? "exact" : "DIFFERS");
	printf("string   \"%s\" %s\n", s_out, strcmp(s_out, s_in) ? "DIFFERS" : "exact");
	pvm_exit();
	printf("%s\n", ok ? "PASS" : "FAIL");
	return !ok;
}
