#include "CPU.h"
#include <chrono>

vCPU::vCPU() {
	// ...
}

V<vClass> vCPU::load(vContext* ctx, const std::string& s, const std::string& parent) {
	auto it = _classes.find(s);
	if (it != _classes.end()) return it->second;
	return _classes[s] = VMAKE(vClass, ctx, ctx, (parent + s + ".class").c_str());
}

bool vCPU::active() const {
	return _running;
}

void vCPU::addNative(const std::string& path, const std::string& desc, const vNATIVE& native)
{
	_natives[path + ":" + desc] = native;
}

void vCPU::run(vContext* ctx, const V<vFrame>& frame) {
	_running = true;
	while (active()) {
		execute(ctx, frame);
	}
	_running = true;
}

size_t vCPU::execute(vContext* ctx, const V<vFrame>& frame) {
	size_t step = sub_execute(ctx, frame);
	frame.Ptr(ctx)->incrpc(step + 1);
	return step;
}

size_t vCPU::sub_execute(vContext* ctx, const V<vFrame>& frame) {
	vStack* _stack = frame.Ptr(ctx)->_stack.Real(ctx);
	vMemory* _local = frame.Ptr(ctx)->_local.Real(ctx);
	vClass* _class = frame.Ptr(ctx)->_class.Real(ctx);
	vMemory* _constPool = frame.Ptr(ctx)->_class.Ptr(ctx)->_constPool.Real(ctx);
	vBYTE* ip = frame.Ptr(ctx)->fetch(ctx);
	// printf("Execute instruction %d (%s)\n", *ip, Opcodes[*ip]);
	vBYTE& opcode = *ip;
	vCOMMON op[8];
	switch (opcode) {
	case nop:
		return 0;
	case iconst_0:
		_stack->push<vINT>(ctx, 0);
		return 0;
	case iconst_1:
		_stack->push<vINT>(ctx, 1);
		return 0;
	case iconst_2:
		_stack->push<vINT>(ctx, 2);
		return 0;
	case iconst_3:
		_stack->push<vINT>(ctx, 3);
		return 0;
	case iconst_4:
		_stack->push<vINT>(ctx, 4);
		return 0;
	case iconst_5:
		_stack->push<vINT>(ctx, 5);
		return 0;
	case iconst_m1:
		_stack->push<vINT>(ctx, -1);
		return 0;
	case lconst_0:
		_stack->push<vLONG>(ctx, 0);
		return 0;
	case lconst_1:
		_stack->push<vLONG>(ctx, 1);
		return 0;
	case dconst_0:
		_stack->push<vDOUBLE>(ctx, 0.0);
		return 0;
	case dconst_1:
		_stack->push<vDOUBLE>(ctx, 1.0);
		return 0;
	case fconst_0:
		_stack->push<vFLOAT>(ctx, 0.0f);
		return 0;
	case fconst_1:
		_stack->push<vFLOAT>(ctx, 1.0f);
		return 0;
	case fconst_2:
		_stack->push<vFLOAT>(ctx, 2.0f);
		return 0;
	case aconst_null:
		_stack->push<vREF>(ctx, vREF{ 0 });
		return 0;

	case bipush:
		_stack->push<vBYTE>(ctx, read<vBYTE>(ip + 1));
		return 1;
	case sipush:
		op[0].usi = readUSI(ip + 1);
		_stack->push<vUSHORT>(ctx, op[0].usi);
		return 2;
	case dup:
		op[0] = _stack->pop<vCOMMON>(ctx);
		_stack->push<vCOMMON>(ctx, op[0]);
		_stack->push<vCOMMON>(ctx, op[0]);
		return 0;

	case i2b:
		_stack->push<vBYTE>(ctx, (vBYTE)(_stack->pop<vUINT>(ctx) & 255));
		return 0;
	case i2c:
		_stack->push<vJCHAR>(ctx, (vJCHAR)(_stack->pop<vUINT>(ctx) & 65535));
		return 0;
	case i2s:
		_stack->push<vUSHORT>(ctx, (vUSHORT)(_stack->pop<vUINT>(ctx) & 65535));
		return 0;
	case i2l:
		_stack->push<vLONG>(ctx, (vLONG)(_stack->pop<vINT>(ctx)));
		return 0;
	case i2f:
		_stack->push<vFLOAT>(ctx, (vFLOAT)(_stack->pop<vINT>(ctx)));
		return 0;
	case i2d:
		_stack->push<vDOUBLE>(ctx, (vDOUBLE)(_stack->pop<vINT>(ctx)));
		return 0;

	case l2i:
		_stack->push<vINT>(ctx, (vINT)_stack->pop<vLONG>(ctx));
		return 0;
	case l2f:
		_stack->push<vFLOAT>(ctx, (vFLOAT)_stack->pop<vLONG>(ctx));
		return 0;
	case l2d:
		_stack->push<vDOUBLE>(ctx, (vDOUBLE)_stack->pop<vLONG>(ctx));
		return 0;

	case f2d:
		_stack->push<vDOUBLE>(ctx, (vDOUBLE)_stack->pop<vFLOAT>(ctx));
		return 0;
	case f2i:
		_stack->push<vINT>(ctx, (vINT)_stack->pop<vFLOAT>(ctx));
		return 0;
	case f2l:
		_stack->push<vLONG>(ctx, (vLONG)_stack->pop<vFLOAT>(ctx));
		return 0;

	case d2f:
		_stack->push<vFLOAT>(ctx, (vFLOAT)_stack->pop<vDOUBLE>(ctx));
		return 0;
	case d2i:
		_stack->push<vINT>(ctx, (vINT)_stack->pop<vDOUBLE>(ctx));
		return 0;
	case d2l:
		_stack->push<vLONG>(ctx, (vLONG)_stack->pop<vDOUBLE>(ctx));
		return 0;

	case ineg:
		op[0].i = _stack->pop<vINT>(ctx);
		_stack->push<vINT>(ctx, -op[0].i);
		return 0;
	case lneg:
		op[0].l = _stack->pop<vLONG>(ctx);
		_stack->push<vLONG>(ctx, -op[0].l);
		return 0;
	case fneg:
		op[0].f = _stack->pop<vFLOAT>(ctx);
		_stack->push<vFLOAT>(ctx, -op[0].f);
		return 0;
	case dneg:
		op[0].d = _stack->pop<vDOUBLE>(ctx);
		_stack->push<vDOUBLE>(ctx, -op[0].d);
		return 0;
		
	case iadd:
		op[1].i = _stack->pop<vINT>(ctx);
		op[0].i = _stack->pop<vINT>(ctx);
		_stack->push<vINT>(ctx, op[0].i + op[1].i);
		return 0;
	case ladd:
		op[1].l = _stack->pop<vLONG>(ctx);
		op[0].l = _stack->pop<vLONG>(ctx);
		_stack->push<vLONG>(ctx, op[0].l + op[1].l);
		return 0;
	case fadd:
		op[1].f = _stack->pop<vFLOAT>(ctx);
		op[0].f = _stack->pop<vFLOAT>(ctx);
		_stack->push<vFLOAT>(ctx, op[0].f + op[1].f);
		return 0;
	case dadd:
		op[1].d = _stack->pop<vDOUBLE>(ctx);
		op[0].d = _stack->pop<vDOUBLE>(ctx);
		_stack->push<vDOUBLE>(ctx, op[0].d + op[1].d);
		return 0;

	case isub:
		op[1].i = _stack->pop<vINT>(ctx);
		op[0].i = _stack->pop<vINT>(ctx);
		_stack->push<vINT>(ctx, op[0].i - op[1].i);
		return 0;
	case lsub:
		op[1].l = _stack->pop<vLONG>(ctx);
		op[0].l = _stack->pop<vLONG>(ctx);
		_stack->push<vLONG>(ctx, op[0].l - op[1].l);
		return 0;
	case fsub:
		op[1].f = _stack->pop<vFLOAT>(ctx);
		op[0].f = _stack->pop<vFLOAT>(ctx);
		_stack->push<vFLOAT>(ctx, op[0].f - op[1].f);
		return 0;
	case dsub:
		op[1].d = _stack->pop<vDOUBLE>(ctx);
		op[0].d = _stack->pop<vDOUBLE>(ctx);
		_stack->push<vDOUBLE>(ctx, op[0].d - op[1].d);
		return 0;

	case idiv:
		op[1].i = _stack->pop<vINT>(ctx);
		op[0].i = _stack->pop<vINT>(ctx);
		_stack->push<vINT>(ctx, op[0].i / op[1].i);
		return 0;
	case ldiv_:
		op[1].l = _stack->pop<vLONG>(ctx);
		op[0].l = _stack->pop<vLONG>(ctx);
		_stack->push<vLONG>(ctx, op[0].l / op[1].l);
		return 0;
	case fdiv:
		op[1].f = _stack->pop<vFLOAT>(ctx);
		op[0].f = _stack->pop<vFLOAT>(ctx);
		_stack->push<vFLOAT>(ctx, op[0].f / op[1].f);
		return 0;
	case ddiv:
		op[1].d = _stack->pop<vDOUBLE>(ctx);
		op[0].d = _stack->pop<vDOUBLE>(ctx);
		_stack->push<vDOUBLE>(ctx, op[0].d / op[1].d);
		return 0;

	case imul:
		op[1].i = _stack->pop<vINT>(ctx);
		op[0].i = _stack->pop<vINT>(ctx);
		_stack->push<vINT>(ctx, op[0].i * op[1].i);
		return 0;
	case lmul:
		op[1].l = _stack->pop<vLONG>(ctx);
		op[0].l = _stack->pop<vLONG>(ctx);
		_stack->push<vLONG>(ctx, op[0].l * op[1].l);
		return 0;
	case fmul:
		op[1].f = _stack->pop<vFLOAT>(ctx);
		op[0].f = _stack->pop<vFLOAT>(ctx);
		_stack->push<vFLOAT>(ctx, op[0].f * op[1].f);
		return 0;
	case dmul:
		op[1].d = _stack->pop<vDOUBLE>(ctx);
		op[0].d = _stack->pop<vDOUBLE>(ctx);
		_stack->push<vDOUBLE>(ctx, op[0].d * op[1].d);
		return 0;

	case irem:
		op[1].i = _stack->pop<vINT>(ctx);
		op[0].i = _stack->pop<vINT>(ctx);
		_stack->push<vINT>(ctx, op[0].i % op[1].i);
		return 0;
	case lrem:
		op[1].l = _stack->pop<vLONG>(ctx);
		op[0].l = _stack->pop<vLONG>(ctx);
		_stack->push<vLONG>(ctx, op[0].l % op[1].l);
		return 0;
	case frem:
		op[1].f = _stack->pop<vFLOAT>(ctx);
		op[0].f = _stack->pop<vFLOAT>(ctx);
		_stack->push<vFLOAT>(ctx, std::fmodf(op[0].f, op[1].f));
		return 0;
	case drem:
		op[1].d = _stack->pop<vDOUBLE>(ctx);
		op[0].d = _stack->pop<vDOUBLE>(ctx);
		_stack->push<vDOUBLE>(ctx, std::fmod(op[0].d, op[1].d));
		return 0;

	case ior:
		op[1].i = _stack->pop<vINT>(ctx);
		op[0].i = _stack->pop<vINT>(ctx);
		_stack->push<vINT>(ctx, op[0].i | op[1].i);
		return 0;
	case lor:
		op[1].l = _stack->pop<vLONG>(ctx);
		op[0].l = _stack->pop<vLONG>(ctx);
		_stack->push<vLONG>(ctx, op[0].l | op[1].l);
		return 0;

	case iand:
		op[1].i = _stack->pop<vINT>(ctx);
		op[0].i = _stack->pop<vINT>(ctx);
		_stack->push<vINT>(ctx, op[0].i & op[1].i);
		return 0;
	case land:
		op[1].l = _stack->pop<vLONG>(ctx);
		op[0].l = _stack->pop<vLONG>(ctx);
		_stack->push<vLONG>(ctx, op[0].l & op[1].l);
		return 0;

	case ishr:
		op[1].i = _stack->pop<vINT>(ctx);
		op[0].i = _stack->pop<vINT>(ctx);
		_stack->push<vINT>(ctx, op[0].i >> op[1].i);
		return 0;
	case iushr:
		op[1].u = _stack->pop<vUINT>(ctx);
		op[0].u = _stack->pop<vUINT>(ctx);
		_stack->push<vUINT>(ctx, op[0].u >> op[1].u);
		return 0;
	case lshr:
		op[1].l = _stack->pop<vLONG>(ctx);
		op[0].l = _stack->pop<vLONG>(ctx);
		_stack->push<vLONG>(ctx, op[0].l >> op[1].l);
		return 0;
	case lushr:
		op[1].ul = _stack->pop<vULONG>(ctx);
		op[0].ul = _stack->pop<vULONG>(ctx);
		_stack->push<vLONG>(ctx, op[0].ul >> op[1].ul);
		return 0;

	case ishl:
		op[1].i = _stack->pop<vINT>(ctx);
		op[0].i = _stack->pop<vINT>(ctx);
		_stack->push<vINT>(ctx, op[0].i << op[1].i);
		return 0;
	case lshl:
		op[1].l = _stack->pop<vLONG>(ctx);
		op[0].l = _stack->pop<vLONG>(ctx);
		_stack->push<vLONG>(ctx, op[0].l << op[1].l);
		return 0;

	case ixor:
		op[1].i = _stack->pop<vINT>(ctx);
		op[0].i = _stack->pop<vINT>(ctx);
		_stack->push<vINT>(ctx, op[0].i ^ op[1].i);
		return 0;
	case lxor:
		op[1].l = _stack->pop<vLONG>(ctx);
		op[0].l = _stack->pop<vLONG>(ctx);
		_stack->push<vLONG>(ctx, op[0].l ^ op[1].l);
		return 0;

	// Memory operations
	// Load

	case aload:
		op[0].b = read<vBYTE>(ip + 1);
		_stack->push<vREF>(ctx, _local->get<vREF>(ctx, op[0].b));
		return 1;
	case iload:
		op[0].b = read<vBYTE>(ip + 1);
		_stack->push<vINT>(ctx, _local->get<vINT>(ctx, op[0].b));
		return 1;
	case lload:
		op[0].b = read<vBYTE>(ip + 1);
		_stack->push<vLONG>(ctx, _local->get<vLONG>(ctx, op[0].b));
		return 1;
	case fload:
		op[0].b = read<vBYTE>(ip + 1);
		_stack->push<vFLOAT>(ctx, _local->get<vFLOAT>(ctx, op[0].b));
		return 1;
	case dload:
		op[0].b = read<vBYTE>(ip + 1);
		_stack->push<vDOUBLE>(ctx, _local->get<vDOUBLE>(ctx, op[0].b));
		return 1;

	case aload_0:
		_stack->push<vREF>(ctx, _local->get<vREF>(ctx, (size_t)0));
		return 0;
	case aload_1:
		_stack->push<vREF>(ctx, _local->get<vREF>(ctx, (size_t)1));
		return 0;
	case aload_2:
		_stack->push<vREF>(ctx, _local->get<vREF>(ctx, (size_t)2));
		return 0;
	case aload_3:
		_stack->push<vREF>(ctx, _local->get<vREF>(ctx, (size_t)3));
		return 0;

	case iload_0:
		_stack->push<vINT>(ctx, _local->get<vINT>(ctx, (size_t)0));
		return 0;
	case iload_1:
		_stack->push<vINT>(ctx, _local->get<vINT>(ctx, (size_t)1));
		return 0;
	case iload_2:
		_stack->push<vINT>(ctx, _local->get<vINT>(ctx, (size_t)2));
		return 0;
	case iload_3:
		_stack->push<vINT>(ctx, _local->get<vINT>(ctx, (size_t)3));
		return 0;

	case lload_0:
		_stack->push<vLONG>(ctx, _local->get<vLONG>(ctx, (size_t)0));
		return 0;
	case lload_1:
		_stack->push<vLONG>(ctx, _local->get<vLONG>(ctx, (size_t)1));
		return 0;
	case lload_2:
		_stack->push<vLONG>(ctx, _local->get<vLONG>(ctx, (size_t)2));
		return 0;
	case lload_3:
		_stack->push<vLONG>(ctx, _local->get<vLONG>(ctx, (size_t)3));
		return 0;

	case fload_0:
		_stack->push<vFLOAT>(ctx, _local->get<vFLOAT>(ctx, (size_t)0));
		return 0;
	case fload_1:
		_stack->push<vFLOAT>(ctx, _local->get<vFLOAT>(ctx, (size_t)1));
		return 0;
	case fload_2:
		_stack->push<vFLOAT>(ctx, _local->get<vFLOAT>(ctx, (size_t)2));
		return 0;
	case fload_3:
		_stack->push<vFLOAT>(ctx, _local->get<vFLOAT>(ctx, (size_t)3));
		return 0;

	case dload_0:
		_stack->push<vDOUBLE>(ctx, _local->get<vDOUBLE>(ctx, (size_t)0));
		return 0;
	case dload_1:
		_stack->push<vDOUBLE>(ctx, _local->get<vDOUBLE>(ctx, (size_t)1));
		return 0;
	case dload_2:
		_stack->push<vDOUBLE>(ctx, _local->get<vDOUBLE>(ctx, (size_t)2));
		return 0;
	case dload_3:
		_stack->push<vDOUBLE>(ctx, _local->get<vDOUBLE>(ctx, (size_t)3));
		return 0;

	// Store
	case astore:
		op[0].b = read<vBYTE>(ip + 1);
		_local->set<vREF>(ctx, (size_t)op[0].b, _stack->pop<vREF>(ctx));
		return 1;
	case istore:
		op[0].b = read<vBYTE>(ip + 1);
		_local->set<vINT>(ctx, (size_t)op[0].b, _stack->pop<vINT>(ctx));
		return 1;
	case lstore:
		op[0].b = read<vBYTE>(ip + 1);
		_local->set<vLONG>(ctx, (size_t)op[0].b, _stack->pop<vLONG>(ctx));
		return 1;
	case fstore:
		op[0].b = read<vBYTE>(ip + 1);
		_local->set<vFLOAT>(ctx, (size_t)op[0].b, _stack->pop<vFLOAT>(ctx));
		return 1;
	case dstore:
		op[0].b = read<vBYTE>(ip + 1);
		_local->set<vDOUBLE>(ctx, (size_t)op[0].b, _stack->pop<vDOUBLE>(ctx));
		return 1;

	case astore_0:
		_local->set<vREF>(ctx, (size_t)0, _stack->pop<vREF>(ctx));
		return 0;
	case astore_1:
		_local->set<vREF>(ctx, (size_t)1, _stack->pop<vREF>(ctx));
		return 0;
	case astore_2:
		_local->set<vREF>(ctx, (size_t)2, _stack->pop<vREF>(ctx));
		return 0;
	case astore_3:
		_local->set<vREF>(ctx, (size_t)3, _stack->pop<vREF>(ctx));
		return 0;

	case istore_0:
		_local->set<vINT>(ctx, (size_t)0, _stack->pop<vINT>(ctx));
		return 0;
	case istore_1:
		_local->set<vINT>(ctx, (size_t)1, _stack->pop<vINT>(ctx));
		return 0;
	case istore_2:
		_local->set<vINT>(ctx, (size_t)2, _stack->pop<vINT>(ctx));
		return 0;
	case istore_3:
		_local->set<vINT>(ctx, (size_t)3, _stack->pop<vINT>(ctx));
		return 0;

	case lstore_0:
		_local->set<vLONG>(ctx, (size_t)0, _stack->pop<vLONG>(ctx));
		return 0;
	case lstore_1:
		_local->set<vLONG>(ctx, (size_t)1, _stack->pop<vLONG>(ctx));
		return 0;
	case lstore_2:
		_local->set<vLONG>(ctx, (size_t)2, _stack->pop<vLONG>(ctx));
		return 0;
	case lstore_3:
		_local->set<vLONG>(ctx, (size_t)3, _stack->pop<vLONG>(ctx));
		return 0;

	case fstore_0:
		_local->set<vFLOAT>(ctx, (size_t)0, _stack->pop<vFLOAT>(ctx));
		return 0;
	case fstore_1:
		_local->set<vFLOAT>(ctx, (size_t)1, _stack->pop<vFLOAT>(ctx));
		return 0;
	case fstore_2:
		_local->set<vFLOAT>(ctx, (size_t)2, _stack->pop<vFLOAT>(ctx));
		return 0;
	case fstore_3:
		_local->set<vFLOAT>(ctx, (size_t)3, _stack->pop<vFLOAT>(ctx));
		return 0;

	case dstore_0:
		_local->set<vDOUBLE>(ctx, (size_t)0, _stack->pop<vDOUBLE>(ctx));
		return 0;
	case dstore_1:
		_local->set<vDOUBLE>(ctx, (size_t)1, _stack->pop<vDOUBLE>(ctx));
		return 0;
	case dstore_2:
		_local->set<vDOUBLE>(ctx, (size_t)2, _stack->pop<vDOUBLE>(ctx));
		return 0;
	case dstore_3:
		_local->set<vDOUBLE>(ctx, (size_t)3, _stack->pop<vDOUBLE>(ctx));
		return 0;

	case ldc:
		op[0].b = read<vBYTE>(ip + 1);
		_stack->push<vCOMMON>(ctx, _constPool->get<vCOMMON>(ctx, (size_t)op[0].b));
		return 1;
	case ldc_w:
	case ldc2_w:
		op[0].usi = readUSI(ip + 1);
		_stack->push<vCOMMON>(ctx, _constPool->get<vCOMMON>(ctx, (size_t)op[0].usi));
		return 2;

	case iinc:
		op[0].b = read<vBYTE>(ip + 1);
		op[1].b = read<vBYTE>(ip + 2);
		_local->set<vINT>(ctx, op[0].b, _local->get<vINT>(ctx, op[0].b) + op[1].b);
		return 2;

	case goto_:
		op[0].usi = readUSI(ip + 1);
		return ((size_t)(op[0].si)) - 1;
	case goto_w:
		op[0].u = readUI(ip + 1);
		return ((size_t)(op[0].i)) - 1;
	case ifeq:
		op[0].usi = readUSI(ip + 1);
		op[1].i = _stack->pop<vINT>(ctx);
		if (op[1].i == 0)
			return ((size_t)(op[0].si)) - 1;
		else return 2;
	case ifne:
		op[0].usi = readUSI(ip + 1);
		op[1].i = _stack->pop<vINT>(ctx);
		if (op[1].i != 0)
			return ((size_t)(op[0].si)) - 1;
		else return 2;
	case ifnull:
		op[0].usi = readUSI(ip + 1);
		op[1].a = _stack->pop<vREF>(ctx);
		if (op[1].a.a == 0)
			return ((size_t)(op[0].si)) - 1;
		else return 2;
	case ifnonnull:
		op[0].usi = readUSI(ip + 1);
		op[1].a = _stack->pop<vREF>(ctx);
		if (op[1].a.a != 0)
			return ((size_t)(op[0].si)) - 1;
		else return 2;

	case iflt:
		op[0].usi = readUSI(ip + 1);
		op[1].i = _stack->pop<vINT>(ctx);
		if (op[1].i < 0)
			return ((size_t)(op[0].si)) - 1;
		else return 2;
	case ifgt:
		op[0].usi = readUSI(ip + 1);
		op[1].i = _stack->pop<vINT>(ctx);
		if (op[1].i > 0)
			return ((size_t)(op[0].si)) - 1;
		else return 2;
	case ifle:
		op[0].usi = readUSI(ip + 1);
		op[1].i = _stack->pop<vINT>(ctx);
		if (op[1].i <= 0)
			return ((size_t)(op[0].si)) - 1;
		else return 2;
	case ifge:
		op[0].usi = readUSI(ip + 1);
		op[1].i = _stack->pop<vINT>(ctx);
		if (op[1].i >= 0)
			return ((size_t)(op[0].si)) - 1;
		else return 2;

	case if_icmpeq:
		op[0].usi = readUSI(ip + 1);
		op[2].i = _stack->pop<vINT>(ctx);
		op[1].i = _stack->pop<vINT>(ctx);
		if (op[1].i == op[1].i)
			return ((size_t)(op[0].si)) - 1;
		else return 2;
	case if_icmpne:
		op[0].usi = readUSI(ip + 1);
		op[2].i = _stack->pop<vINT>(ctx);
		op[1].i = _stack->pop<vINT>(ctx);
		if (op[1].i != op[2].i)
			return ((size_t)(op[0].si)) - 1;
		else return 2;
	case if_icmplt:
		op[0].usi = readUSI(ip + 1);
		op[2].i = _stack->pop<vINT>(ctx);
		op[1].i = _stack->pop<vINT>(ctx);
		if (op[1].i < op[2].i)
			return ((size_t)(op[0].si)) - 1;
		else return 2;
	case if_icmpgt:
		op[0].usi = readUSI(ip + 1);
		op[2].i = _stack->pop<vINT>(ctx);
		op[1].i = _stack->pop<vINT>(ctx);
		if (op[1].i > op[2].i)
			return ((size_t)(op[0].si)) - 1;
		else return 2;
	case if_icmple:
		op[0].usi = readUSI(ip + 1);
		op[2].i = _stack->pop<vINT>(ctx);
		op[1].i = _stack->pop<vINT>(ctx);
		if (op[1].i <= op[2].i)
			return ((size_t)(op[0].si)) - 1;
		else return 2;
	case if_icmpge:
		op[0].usi = readUSI(ip + 1);
		op[2].i = _stack->pop<vINT>(ctx);
		op[1].i = _stack->pop<vINT>(ctx);
		if (op[1].i >= op[2].i)
			return ((size_t)(op[0].si)) - 1;
		else return 2;
	case if_acmpeq:
		op[0].usi = readUSI(ip + 1);
		op[2].a = _stack->pop<vREF>(ctx);
		op[1].a = _stack->pop<vREF>(ctx);
		if (op[1].a.a == op[2].a.a)
			return ((size_t)(op[0].si)) - 1;
		else return 2;
	case if_acmpne:
		op[0].usi = readUSI(ip + 1);
		op[2].a = _stack->pop<vREF>(ctx);
		op[1].a = _stack->pop<vREF>(ctx);
		if (op[1].a.a != op[2].a.a)
			return ((size_t)(op[0].si)) - 1;
		else return 2;

	case lcmp:
		op[1].l = _stack->pop<vLONG>(ctx);
		op[0].l = _stack->pop<vLONG>(ctx);
		_stack->push<vINT>(ctx, compare(op[0].l, op[0].l));
		return 0;
	case fcmpg:
	case fcmpl:
		op[1].f = _stack->pop<vFLOAT>(ctx);
		op[0].f = _stack->pop<vFLOAT>(ctx);
		_stack->push<vINT>(ctx, compare(op[0].f, op[0].f));
		return 0;
	case dcmpg:
	case dcmpl:
		op[1].d = _stack->pop<vDOUBLE>(ctx);
		op[0].d = _stack->pop<vDOUBLE>(ctx);
		_stack->push<vINT>(ctx, compare(op[0].d, op[0].d));
		return 0;

	case return_:
		_running = false;
		return 0;

	case ireturn:
	case lreturn:
	case dreturn:
	case freturn:
	case areturn:
		_running = false;
		return 0;

	case invokevirtual:
	case invokestatic:
	case invokespecial:
	{
		op[0].usi = readUSI(ip + 1);
		op[1].mr = _constPool->get<vMETHODREF>(ctx, (size_t)op[0].usi);
		std::string path = std::string((const char*)_class->toString(ctx, op[1].mr.clsIndex).Ptr(ctx)->s.Real(ctx));
		std::string methodName = (const char*)_class->toString(ctx, op[1].mr.nameIndex).Ptr(ctx)->s.Real(ctx);
		std::string desc = (const char*)_class->toString(ctx, op[1].mr.nameIndex, 1).Ptr(ctx)->s.Real(ctx);
		//printf("%s: %s\n", Opcodes[opcode], (path + "/" + methodName + desc).c_str());
		auto nit = _natives.find(path + "/" + methodName + ":" + desc);
		bool found = false;
		if (nit != _natives.end()) {
			nit->second(ctx, path, this, _stack, opcode);
			found = true;
		} else {
			auto it = _classes.find(path);
			if (it != _classes.end()) {
				V<vClass> cls = it->second;
				auto [methodFound, ret] = cls.Ptr(ctx)->invoke(ctx, cls, this, _stack, opcode, methodName, desc);
				found = methodFound;
			}
		}
		if(!found) {
			fprintf(stderr, "[vCPU::sub_execute/%s] Couldn't find virtual %s (%s)\n", Opcodes[opcode], (path + "/" + methodName).c_str(), desc.c_str());
			_running = false;
		}
		return 2;
	}
	case new_:
	{
		op[0].usi = readUSI(ip + 1);
		op[1].mr = _constPool->get<vMETHODREF>(ctx, (size_t)op[0].usi);
		std::string path = std::string((const char*)_class->toString(ctx, op[1].mr.clsIndex).Ptr(ctx)->s.Real(ctx));
		auto it = _classes.find(path);
		bool found = false;
		if (it != _classes.end()) {
			V<vClass> cls = it->second;
			V<vOBJECT> obj = VMAKE(vOBJECT, ctx, ctx, cls);
			vOBJECTREF ref; ref.r.a = (vULONG)obj.Virtual(ctx);
			_stack->push<vOBJECTREF>(ctx, ref);
			found = true;
		}
		if (!found) {
			fprintf(stderr, "[vCPU::sub_execute/%s] Couldn't find class %s\n", Opcodes[opcode], path.c_str());
			_running = false;
		}
		return 2;
	}
	case getstatic:
	{
		op[0].usi = readUSI(ip + 1);
		op[3].mr = _constPool->get<vMETHODREF>(ctx, op[0].usi);
		std::string path = std::string((const char*)_class->toString(ctx, op[3].mr.clsIndex).Ptr(ctx)->s.Real(ctx));
		std::string field = std::string((const char*)_class->toString(ctx, op[3].mr.nameIndex).Ptr(ctx)->s.Real(ctx));
		auto it = _classes.find(path);
		bool found = false;
		if (it != _classes.end()) {
			V<vClass> cls = it->second;
			if (!cls.Ptr(ctx)->_initialized) {
				cls.Ptr(ctx)->_initialized = true;
				cls.Ptr(ctx)->invoke(ctx, cls, this, _stack, invokestatic, "<clinit>", "()V");
			}
			vFIELD* fld = cls.Ptr(ctx)->getField(ctx, field.c_str());
			if (fld) {
				_stack->push<vCOMMON>(ctx, fld->value);
				found = true;
			}
		}
		if(!found){
			vCOMMON dummy;
			memset(&dummy, 0, sizeof(vCOMMON));
			_stack->push<vCOMMON>(ctx, dummy);
		}
		return 2;
	}
	case putstatic:
	{
		op[0].usi = readUSI(ip + 1);
		op[3].mr = _constPool->get<vMETHODREF>(ctx, op[0].usi);
		std::string path = std::string((const char*)_class->toString(ctx, op[3].mr.clsIndex).Ptr(ctx)->s.Real(ctx));
		std::string field = std::string((const char*)_class->toString(ctx, op[3].mr.nameIndex).Ptr(ctx)->s.Real(ctx));
		auto it = _classes.find(path);
		bool found = false;
		if (it != _classes.end()) {
			V<vClass> cls = it->second;
			if (!cls.Ptr(ctx)->_initialized) {
				cls.Ptr(ctx)->_initialized = true;
				cls.Ptr(ctx)->invoke(ctx, cls, this, _stack, invokestatic, "<clinit>", "()V");
			}
			vFIELD* fld = cls.Ptr(ctx)->getField(ctx, field.c_str());
			if (fld) {
				fld->value = _stack->pop<vCOMMON>(ctx);
				found = true;
			}
		}
		if (!found) {
			_stack->pop<vCOMMON>(ctx);
		}
		return 2;
	}
	case getfield:
	{
		op[0].usi = readUSI(ip + 1);
		op[1].objref = _stack->pop<vOBJECTREF>(ctx);
		op[3].mr = _constPool->get<vMETHODREF>(ctx, (size_t)op[0].usi);
		V<vOBJECT> obj((vOBJECT*)op[1].objref.r.a);
		vUSHORT fieldIdx = obj.Ptr(ctx)->cls.Ptr(ctx)->_fieldLookup[VCtxIdx{ ctx, (size_t)op[3].mr.nameIndex }];
		bool found = fieldIdx != 0xFFFF;
		if (found) {
			_stack->push<vCOMMON>(ctx, obj.Ptr(ctx)->fields[VCtxIdx{ ctx, (size_t)fieldIdx }]);
		} else {
			fprintf(stderr, "[vCPU::sub_execute/%s] Couldn't find field index %d\n", Opcodes[opcode], op[3].mr.nameIndex);
			_running = false;
		}
		return 2;
	}
	case putfield:
	{
		op[0].usi = readUSI(ip + 1);
		op[2] = _stack->pop<vCOMMON>(ctx);
		op[1].objref = _stack->pop<vOBJECTREF>(ctx);
		op[3].mr = _constPool->get<vMETHODREF>(ctx, (size_t)op[0].usi);
		V<vOBJECT> obj((vOBJECT*)op[1].objref.r.a);
		V<vUTF8BODY> fieldName = obj.Ptr(ctx)->cls.Ptr(ctx)->toString(ctx, op[3].mr.nameIndex);
		vUSHORT fieldIdx = obj.Ptr(ctx)->cls.Ptr(ctx)->_fieldLookup[VCtxIdx{ ctx, (size_t)op[3].mr.nameIndex }];
		bool found = fieldIdx != 0xFFFF;
		if (found) {
			obj.Ptr(ctx)->fields[VCtxIdx{ ctx, fieldIdx }] = op[2];
		} else {
			fprintf(stderr, "[vCPU::sub_execute/%s] Couldn't find field index %d\n", Opcodes[opcode], op[3].mr.nameIndex);
			_running = false;
		}
		return 2;
	}
	case pop:
		_stack->pop<vCOMMON>(ctx);
		return 0;
	default:
		fprintf(stderr, "[vCPU::sub_execute] Executing undefined instruction with opcode %d (%s)\n", *ip, Opcodes[*ip]);
		_running = false;
		return 0;
	}
}

vUSHORT vCPU::readUSI(vBYTE* ip) const {
	vUSHORT usi = read<vBYTE>(ip);
	usi <<= 8;
	usi |= read<vBYTE>(ip + 1);
	return usi;
}

vUINT vCPU::readUI(vBYTE* ip) const {
	vUINT ui = read<vBYTE>(ip);
	ui <<= 8; ui |= read<vBYTE>(ip + 1);
	ui <<= 8; ui |= read<vBYTE>(ip + 2);
	ui <<= 8; ui |= read<vBYTE>(ip + 3);
	return ui;
}

int main(int argc, const char** argv) {

	const char* DirPath = "Assets/";
	const char* ClsPath = "Main";
	const char* MethodPath = "main";

	if (argc >= 2) ClsPath = argv[1];
	if (argc >= 3) DirPath = argv[2];
	if (argc >= 4) MethodPath = argv[3];

	vContext vCtx;
	vContext* ctx = &vCtx;

	auto cpu = VMAKE(vCPU, ctx); // vCPU();
	auto cls = cpu.Ptr(ctx)->load(ctx, ClsPath, DirPath);
	auto frame = VMAKE(vFrame, ctx, ctx, cls.Ptr(ctx)->getMethod(ctx, MethodPath), cls);

	cpu.Ptr(ctx)->addNative("java/lang/Object/<init>", "()V", [](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
		//printf("Initialize class %s, opcode: %s\n", cls.c_str(), Opcodes[opcode]);
		if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
	});

	cpu.Ptr(ctx)->addNative("java/io/PrintStream/println", "(I)V", [](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
		vINT arg = stack->pop<vINT>(ctx);
		if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		printf("%d\n", arg);
	});


	cpu.Ptr(ctx)->addNative("java/io/PrintStream/println", "(J)V", [](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
		vLONG arg = stack->pop<vLONG>(ctx);
		if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		printf("%lld\n", arg);
	});

	cpu.Ptr(ctx)->addNative("java/lang/System/currentTimeMillis", "()J", [](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
		if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		);
		vLONG millis = (vLONG)ms.count();
		stack->push<vLONG>(ctx, millis);
	});

	cpu.Ptr(ctx)->run(ctx, frame);
	//printf("Result: %d, type: %d\n", retval.i, retval.type);
}