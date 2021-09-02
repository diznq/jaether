#include "CPU.h"

using namespace jaether;

int main(int argc, const char** argv) {
#ifdef _WIN32
	system("@chcp 65001>nul");
#endif

	const char* ClsPath = "example/Main";
	const char* MethodPath = "main";
	bool SecureContext = false;

	if (argc >= 2) ClsPath = argv[1];
	if (argc >= 3) MethodPath = argv[2];
	if (argc >= 4) SecureContext = argv[3][0] == '1';

	vContext* ctx = new vContext(64 * 1024 * 1024, SecureContext);

	vCPU* cpu = new vCPU();
	auto cls = cpu->load(ctx, ClsPath);
	auto frame = VMAKE(vFrame, ctx, ctx, cls.Ptr(ctx)->getMethod(ctx, MethodPath), cls);

	auto start = cpu->getTime();
	cpu->run(ctx, frame);
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(cpu->getTime() - start).count();
	double duration_s = duration / 1000000.0;
	unsigned char sig[64];
	ctx->GetSignature(sig);
	printf("--------Execution info--------\n");
	printf("Execution time          : %.3f ms\n", duration_s * 1000);
	printf("VM Ops                  : %lld\n", ctx->Ops());
	printf("VM kOps/s               : %.3f\n", ctx->Ops() / duration_s / 1000.0);
	printf("Alloc c/a               : %lld\n", ctx->GetAllocator()->GetAvgCycles());
	printf("Currently managed memory: %lld\n", ctx->GetAllocator()->GetManagedSize());
	printf("Peak managed memory     : %lld\n", ctx->GetAllocator()->GetPeakSize());
	printf("Signature               : ");
	for (int i = 0; i < 32; i++) {
		printf("%02x", sig[i] & 255);
	}
	printf("\n");
	//printf("Result: %d, type: %d\n", retval.i, retval.type);
}