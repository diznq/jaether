#include "CPU.h"

using namespace jaether;

int main(int argc, const char** argv) {

	const char* ClsPath = "Main";
	const char* MethodPath = "main";
	bool SecureContext = false;

	if (argc >= 2) ClsPath = argv[1];
	if (argc >= 3) MethodPath = argv[2];
	if (argc >= 4) SecureContext = argv[3][0] == '1';

	vContext* ctx = new vContext(64 * 1024 * 1024, SecureContext);

	vCPU* cpu = new vCPU();
	cpu->load(ctx, "java/lang/String");
	cpu->load(ctx, "java/lang/String$CaseInsensitiveComparator");
	cpu->load(ctx, "java/lang/StringLatin1");
	cpu->load(ctx, "java/lang/Math");
	cpu->load(ctx, "java/util/Arrays");
	auto cls = cpu->load(ctx, ClsPath);
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

	cpu->addNative("java/lang/System/arraycopy", "(Ljava/lang/Object;ILjava/lang/Object;II)V", [](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
		vINT len = stack->pop<vINT>(ctx);
		vINT dstPos = stack->pop<vINT>(ctx);
		vOBJECTREF dst = stack->pop<vOBJECTREF>(ctx);
		vINT srcPos = stack->pop<vINT>(ctx);
		vOBJECTREF src = stack->pop<vOBJECTREF>(ctx);

		V<vNATIVEARRAY> srcArr((vNATIVEARRAY*)src.r.a);
		V<vNATIVEARRAY> dstArr((vNATIVEARRAY*)dst.r.a);

		auto unit = srcArr.Ptr(ctx)->unitSize(srcArr.Ptr(ctx)->type);
		vBYTE* pSrc = srcArr.Ptr(ctx)->data.Ptr(ctx);
		vBYTE* pDst = dstArr.Ptr(ctx)->data.Ptr(ctx);

		memmove(pDst + dstPos * unit, pSrc + srcPos * unit, len * unit);

		if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
	});

	cpu->addNative("java/lang/System/registerNatives", "()V", [](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
		if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		printf("Natives registered\n");
	});

	cpu->addNative("java/io/PrintStream/println", "(Ljava/lang/String;)V", [](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
		vOBJECTREF arg = stack->pop<vOBJECTREF>(ctx);
		V<vOBJECT> vobj((vOBJECT*)arg.r.a);
		vOBJECT* obj = vobj.Ptr(ctx);
		vClass* klass = obj->cls.Ptr(ctx);
		vCOMMON value = obj->fields[VCtxIdx{ ctx, 0 }];
		V<vNATIVEARRAY> varr((vNATIVEARRAY*)value.a.a);
		vNATIVEARRAY* arr = varr.Ptr(ctx);
		vJCHAR* data = (vJCHAR*)arr->data.Ptr(ctx);
		char* cdata = (char*)data;
		fwrite(cdata, 1, arr->size, stdout);
		fputc('\n', stdout);
		fflush(stdout);
		if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
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