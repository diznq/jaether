#include "CPU.h"

using namespace jaether;

int main(int argc, const char** argv) {

	const char* DirPath = "Assets/";
	const char* ClsPath = "Main";
	const char* MethodPath = "main";
	bool SecureContext = false;

	if (argc >= 2) ClsPath = argv[1];
	if (argc >= 3) DirPath = argv[2];
	if (argc >= 4) MethodPath = argv[3];
	if (argc >= 5) SecureContext = argv[4][0] == '1';

	vContext* ctx = new vContext(64 * 1024 * 1024, SecureContext);

	vCPU* cpu = new vCPU();
	auto cls = cpu->load(ctx, ClsPath, DirPath);
	auto frame = VMAKE(vFrame, ctx, ctx, cls.Ptr(ctx)->getMethod(ctx, MethodPath), cls);

	cpu->addNative("java/lang/Object/<init>", "()V", [](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
		//printf("Initialize class %s, opcode: %s\n", cls.c_str(), Opcodes[opcode]);
		if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		});

	cpu->addNative("java/io/PrintStream/println", "(I)V", [](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
		vINT arg = stack->pop<vINT>(ctx);
		if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		printf("%d\n", arg);
		});


	cpu->addNative("java/io/PrintStream/println", "(J)V", [](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
		vLONG arg = stack->pop<vLONG>(ctx);
		if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		printf("%lld\n", arg);
		});

	cpu->addNative("java/lang/System/currentTimeMillis", "()J", [](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
		if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
			);
		vLONG millis = (vLONG)ms.count();
		stack->push<vLONG>(ctx, ctx->Ops() / 50000);
		});

	auto start = cpu->getTime();
	cpu->run(ctx, frame);
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(cpu->getTime() - start).count();
	double duration_s = duration / 1000000.0;
	unsigned char sig[64];
	ctx->GetSignature(sig);
	printf("--------Execution info--------\n");
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