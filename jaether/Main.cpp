#include "CPU.h"
#include <filesystem>
using namespace jaether;

int main(int argc, const char** argv) {
#ifdef _WIN32
	system("@chcp 65001>nul");
#endif

	const char* ClsPath = "example/Main";
	const char* MethodPath = "main";
	bool SecureContext = false;
	bool HotLoading = false;
	bool FullInit = true;
	
	for (int i = 1; i < argc; i++) {
		if (!strncmp(argv[i], "--", 2) && (i + 1) < argc) {
			const char* K = argv[i];
			const char* V = argv[i + 1];
			i++;
			if (!strcmp(K, "--class")) ClsPath = V;
			else if (!strcmp(K, "--entry")) MethodPath = V;
			else if (!strcmp(K, "--secure")) SecureContext = V[0] == '1';
			else if (!strcmp(K, "--hot")) HotLoading = V[0] == '1';
			else if (!strcmp(K, "--init")) FullInit = V[0] == '1';
		}
	}

	if (SecureContext) HotLoading = true;
	if (HotLoading && std::filesystem::exists("hotload/checkpoint.bin"))
		FullInit = false;


	Allocator* allocator = new Allocator(8 * 1024 * 1024);
	vContext* ctx = new vContext(allocator, FullInit, SecureContext);
	vCPU* cpu = new vCPU();

	if (FullInit) {
		cpu->lazyLoad(ctx, "java/lang/System");
		if (HotLoading) {
			ctx->save("hotload/checkpoint.bin");
		}
	} else if (HotLoading) {
		ctx->load("hotload/checkpoint.bin");
	}

	auto cls = cpu->load(ctx, ClsPath);
	auto frame = VMAKE(vFrame, ctx, ctx, cls(ctx)->getMethod(ctx, MethodPath), cls);

	auto start = cpu->getTime();
	cpu->run(ctx, frame);
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(cpu->getTime() - start).count();
	double duration_s = duration / 1000000.0;
	unsigned char sig[64];
	ctx->getSignature(sig);
	printf("--------Execution info--------\n");
	printf("Execution time          : %.3f ms\n", duration_s * 1000);
	printf("VM Ops                  : %lld\n", ctx->ops());
	printf("VM kOps/s               : %.3f\n", ctx->ops() / duration_s / 1000.0);
	printf("Alloc c/a               : %lld\n", ctx->getAllocator()->getAvgCycles());
	printf("Currently managed memory: %lld\n", ctx->getAllocator()->getManagedSize());
	printf("Peak managed memory     : %lld\n", ctx->getAllocator()->getPeakSize());
	printf("Signature               : ");
	for (int i = 0; i < 32; i++) {
		printf("%02x", sig[i] & 255);
	}
	printf("\n");
	//printf("Result: %d, type: %d\n", retval.i, retval.type);
}