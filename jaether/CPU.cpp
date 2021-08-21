#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <cmath>
#include "Opcodes.h"
#include "Pointer.h"
#include "Types.h"
#include "Stack.h"
#include "Memory.h"

class vFrame {
public:
	V<vBYTE> _program;
	V<vStack> _stack;
	V<vMemory> _local;
	V<vMemory> _constPool;
	V<vULONG> _pc;

	vFrame(const V<vBYTE>& program = 0) {
		_stack = VMAKE(vStack);
		_local = VMAKE(vMemory);
		_constPool = VMAKE(vMemory);
		_pc = VMAKE(vULONG);
		_program = program;
	}

	~vFrame() {
		delete _stack.Real();
		delete _pc.Real();
		delete _local.Real();
		delete _constPool.Real();
	}

	vBYTE* fetch() const {
		return _program.Real() + pc();
	}

	vULONG& pc() const {
		return *_pc;
	}

	vULONG incrpc(size_t step) const {
		return (*_pc) += step;
	}
};

class vCPU {
	bool _running = true;
public:
	vCPU() {
		// ...
	}

	bool active() const {
		return _running;
	}

	vCOMMON run(const  V<vFrame>& frame) {
		_running = true;
		vCOMMON retVal;
		memset(&retVal, 0, sizeof(retVal));
		while (active()) {
			execute(frame, retVal);
		}
		return retVal;
	}

	size_t execute(const V<vFrame>& frame, vCOMMON& retVal) {
		size_t step = sub_execute(frame, retVal);
		frame->incrpc(step + 1);
		return step;
	}

	size_t sub_execute(const V<vFrame>& frame, vCOMMON& retVal) {
		V<vStack>& _stack = frame->_stack;
		V<vMemory>& _local = frame->_local;
		V<vMemory>& _constPool = frame->_constPool;
		vBYTE* ip = frame->fetch();
		vBYTE& opcode = *ip;
		vCOMMON op[8];
		switch (opcode) {
		case nop:
			return 0;
		case iconst_0:
			_stack->push<vINT>(0);
			return 0;
		case iconst_1:
			_stack->push<vINT>(1);
			return 0;
		case iconst_2:
			_stack->push<vINT>(2);
			return 0;
		case iconst_3:
			_stack->push<vINT>(3);
			return 0;
		case iconst_4:
			_stack->push<vINT>(4);
			return 0;
		case iconst_5:
			_stack->push<vINT>(5);
			return 0;
		case iconst_m1:
			_stack->push<vINT>(-1);
			return 0;
		case lconst_0:
			_stack->push<vLONG>(0);
			return 0;
		case lconst_1:
			_stack->push<vLONG>(1);
			return 0;
		case dconst_0:
			_stack->push<vDOUBLE>(0.0);
			return 0;
		case dconst_1:
			_stack->push<vDOUBLE>(1.0);
			return 0;
		case fconst_0:
			_stack->push<vFLOAT>(0.0f);
			return 0;
		case fconst_1:
			_stack->push<vFLOAT>(1.0f);
			return 0;
		case fconst_2:
			_stack->push<vFLOAT>(2.0f);
			return 0;
		case bipush:
			_stack->push<vBYTE>(read<vBYTE>(ip + 1));
			return 1;
		case sipush:
			op[0].usi = read<vBYTE>(ip + 1);
			op[0].usi <<= 8;
			op[0].usi |= read<vBYTE>(ip + 2);
			_stack->push<vUSHORT>(op[0].usi);
			return 2;
		case dup:
			op[0] = _stack->pop<vCOMMON>();
			_stack->push(op[0]);
			_stack->push(op[0]);
			return 0;

		case i2b:
			_stack->push<vBYTE>((vBYTE)(_stack->pop<vUINT>() & 255));
			break;
		case i2c:
			_stack->push<vJCHAR>((vJCHAR)(_stack->pop<vUINT>() & 65535));
			break;
		case i2s:
			_stack->push<vUSHORT>((vUSHORT)(_stack->pop<vUINT>() & 65535));
			break;
		case i2l:
			_stack->push<vLONG>((vLONG)(_stack->pop<vINT>()));
			break;
		case i2f:
			_stack->push<vFLOAT>((vFLOAT)(_stack->pop<vINT>()));
			break;
		case i2d:
			_stack->push<vDOUBLE>((vDOUBLE)(_stack->pop<vINT>()));
			break;

		case l2i:
			_stack->push<vINT>((vINT)_stack->pop<vLONG>());
			break;
		case l2f:
			_stack->push<vFLOAT>((vFLOAT)_stack->pop<vLONG>());
			break;
		case l2d:
			_stack->push<vDOUBLE>((vDOUBLE)_stack->pop<vLONG>());
			break;

		case f2d:
			_stack->push<vDOUBLE>((vDOUBLE)_stack->pop<vFLOAT>());
			break;
		case f2i:
			_stack->push<vINT>((vINT)_stack->pop<vFLOAT>());
			break;
		case f2l:
			_stack->push<vLONG>((vLONG)_stack->pop<vFLOAT>());
			break;

		case d2f:
			_stack->push<vFLOAT>((vFLOAT)_stack->pop<vDOUBLE>());
			break;
		case d2i:
			_stack->push<vINT>((vINT)_stack->pop<vDOUBLE>());
			break;
		case d2l:
			_stack->push<vLONG>((vLONG)_stack->pop<vDOUBLE>());
			break;

		case ineg:
			op[0].i = _stack->pop<vINT>();
			_stack->push<vINT>(-op[0].i);
			return 0;
		case lneg:
			op[0].l = _stack->pop<vLONG>();
			_stack->push<vLONG>(-op[0].l);
			return 0;
		case fneg:
			op[0].f = _stack->pop<vFLOAT>();
			_stack->push<vFLOAT>(-op[0].f);
			return 0;
		case dneg:
			op[0].d = _stack->pop<vDOUBLE>();
			_stack->push<vDOUBLE>(-op[0].d);
			return 0;
		
		case iadd:
			op[1].i = _stack->pop<vINT>();
			op[0].i = _stack->pop<vINT>();
			_stack->push<vINT>(op[0].i + op[1].i);
			return 0;
		case ladd:
			op[1].l = _stack->pop<vLONG>();
			op[0].l = _stack->pop<vLONG>();
			_stack->push<vLONG>(op[0].l + op[1].l);
			return 0;
		case fadd:
			op[1].f = _stack->pop<vFLOAT>();
			op[0].f = _stack->pop<vFLOAT>();
			_stack->push<vFLOAT>(op[0].f + op[1].f);
			return 0;
		case dadd:
			op[1].d = _stack->pop<vDOUBLE>();
			op[0].d = _stack->pop<vDOUBLE>();
			_stack->push<vDOUBLE>(op[0].d + op[1].d);
			return 0;

		case isub:
			op[1].i = _stack->pop<vINT>();
			op[0].i = _stack->pop<vINT>();
			_stack->push<vINT>(op[0].i - op[1].i);
			return 0;
		case lsub:
			op[1].l = _stack->pop<vLONG>();
			op[0].l = _stack->pop<vLONG>();
			_stack->push<vLONG>(op[0].l - op[1].l);
			return 0;
		case fsub:
			op[1].f = _stack->pop<vFLOAT>();
			op[0].f = _stack->pop<vFLOAT>();
			_stack->push<vFLOAT>(op[0].f - op[1].f);
			return 0;
		case dsub:
			op[1].d = _stack->pop<vDOUBLE>();
			op[0].d = _stack->pop<vDOUBLE>();
			_stack->push<vDOUBLE>(op[0].d - op[1].d);
			return 0;

		case idiv:
			op[1].i = _stack->pop<vINT>();
			op[0].i = _stack->pop<vINT>();
			_stack->push<vINT>(op[0].i / op[1].i);
			return 0;
		case ldiv_:
			op[1].l = _stack->pop<vLONG>();
			op[0].l = _stack->pop<vLONG>();
			_stack->push<vLONG>(op[0].l / op[1].l);
			return 0;
		case fdiv:
			op[1].f = _stack->pop<vFLOAT>();
			op[0].f = _stack->pop<vFLOAT>();
			_stack->push<vFLOAT>(op[0].f / op[1].f);
			return 0;
		case ddiv:
			op[1].d = _stack->pop<vDOUBLE>();
			op[0].d = _stack->pop<vDOUBLE>();
			_stack->push<vDOUBLE>(op[0].d / op[1].d);
			return 0;

		case imul:
			op[1].i = _stack->pop<vINT>();
			op[0].i = _stack->pop<vINT>();
			printf("imul %d, %d\n", op[1].i, op[0].i);
			_stack->push<vINT>(op[0].i * op[1].i);
			return 0;
		case lmul:
			op[1].l = _stack->pop<vLONG>();
			op[0].l = _stack->pop<vLONG>();
			_stack->push<vLONG>(op[0].l * op[1].l);
			return 0;
		case fmul:
			op[1].f = _stack->pop<vFLOAT>();
			op[0].f = _stack->pop<vFLOAT>();
			_stack->push<vFLOAT>(op[0].f * op[1].f);
			return 0;
		case dmul:
			op[1].d = _stack->pop<vDOUBLE>();
			op[0].d = _stack->pop<vDOUBLE>();
			_stack->push<vDOUBLE>(op[0].d * op[1].d);
			return 0;

		case irem:
			op[1].i = _stack->pop<vINT>();
			op[0].i = _stack->pop<vINT>();
			_stack->push<vINT>(op[0].i % op[1].i);
			return 0;
		case lrem:
			op[1].l = _stack->pop<vLONG>();
			op[0].l = _stack->pop<vLONG>();
			_stack->push<vLONG>(op[0].l % op[1].l);
			return 0;
		case frem:
			op[1].f = _stack->pop<vFLOAT>();
			op[0].f = _stack->pop<vFLOAT>();
			_stack->push<vFLOAT>(std::fmodf(op[0].f, op[1].f));
			return 0;
		case drem:
			op[1].d = _stack->pop<vDOUBLE>();
			op[0].d = _stack->pop<vDOUBLE>();
			_stack->push<vDOUBLE>(std::fmod(op[0].d, op[1].d));
			return 0;

		case ior:
			op[1].i = _stack->pop<vINT>();
			op[0].i = _stack->pop<vINT>();
			_stack->push<vINT>(op[0].i | op[1].i);
			return 0;
		case lor:
			op[1].l = _stack->pop<vLONG>();
			op[0].l = _stack->pop<vLONG>();
			_stack->push<vLONG>(op[0].l | op[1].l);
			return 0;

		case iand:
			op[1].i = _stack->pop<vINT>();
			op[0].i = _stack->pop<vINT>();
			_stack->push<vINT>(op[0].i & op[1].i);
			return 0;
		case land:
			op[1].l = _stack->pop<vLONG>();
			op[0].l = _stack->pop<vLONG>();
			_stack->push<vLONG>(op[0].l & op[1].l);
			return 0;

		case ishr:
			op[1].i = _stack->pop<vINT>();
			op[0].i = _stack->pop<vINT>();
			_stack->push<vINT>(op[0].i >> op[1].i);
			return 0;
		case iushr:
			op[1].u = _stack->pop<vUINT>();
			op[0].u = _stack->pop<vUINT>();
			_stack->push<vUINT>(op[0].u >> op[1].u);
			return 0;
		case lshr:
			op[1].l = _stack->pop<vLONG>();
			op[0].l = _stack->pop<vLONG>();
			_stack->push<vLONG>(op[0].l >> op[1].l);
			return 0;
		case lushr:
			op[1].ul = _stack->pop<vULONG>();
			op[0].ul = _stack->pop<vULONG>();
			_stack->push<vLONG>(op[0].ul >> op[1].ul);
			return 0;

		case ishl:
			op[1].i = _stack->pop<vINT>();
			op[0].i = _stack->pop<vINT>();
			_stack->push<vINT>(op[0].i << op[1].i);
			return 0;
		case lshl:
			op[1].l = _stack->pop<vLONG>();
			op[0].l = _stack->pop<vLONG>();
			_stack->push<vLONG>(op[0].l << op[1].l);
			return 0;

		case ixor:
			op[1].i = _stack->pop<vINT>();
			op[0].i = _stack->pop<vINT>();
			_stack->push<vINT>(op[0].i ^ op[1].i);
			return 0;
		case lxor:
			op[1].l = _stack->pop<vLONG>();
			op[0].l = _stack->pop<vLONG>();
			_stack->push<vLONG>(op[0].l ^ op[1].l);
			return 0;

		// Memory operations
		// Load

		case aload:
			op[0].b = read<vBYTE>(ip + 1);
			_stack->push<vREF>(_local->get<vREF>(op[0].b));
			return 1;
		case iload:
			op[0].b = read<vBYTE>(ip + 1);
			_stack->push<vINT>(_local->get<vINT>(op[0].b));
			return 1;
		case lload:
			op[0].b = read<vBYTE>(ip + 1);
			_stack->push<vLONG>(_local->get<vLONG>(op[0].b));
			return 1;
		case fload:
			op[0].b = read<vBYTE>(ip + 1);
			_stack->push<vFLOAT>(_local->get<vFLOAT>(op[0].b));
			return 1;
		case dload:
			op[0].b = read<vBYTE>(ip + 1);
			_stack->push<vDOUBLE>(_local->get<vDOUBLE>(op[0].b));
			return 1;

		case aload_0:
			_local->set<vREF>((size_t)0, _stack->pop<vREF>());
			return 0;
		case aload_1:
			_local->set<vREF>((size_t)1, _stack->pop<vREF>());
			return 0;
		case aload_2:
			_local->set<vREF>((size_t)2, _stack->pop<vREF>());
			return 0;
		case aload_3:
			_local->set<vREF>((size_t)3, _stack->pop<vREF>());
			return 0;

		case iload_0:
			_local->set<vINT>((size_t)0, _stack->pop<vINT>());
			return 0;
		case iload_1:
			_local->set<vINT>((size_t)1, _stack->pop<vINT>());
			return 0;
		case iload_2:
			_local->set<vINT>((size_t)2, _stack->pop<vINT>());
			return 0;
		case iload_3:
			_local->set<vINT>((size_t)3, _stack->pop<vINT>());
			return 0;

		case lload_0:
			_local->set<vLONG>((size_t)0, _stack->pop<vLONG>());
			return 0;
		case lload_1:
			_local->set<vLONG>((size_t)1, _stack->pop<vLONG>());
			return 0;
		case lload_2:
			_local->set<vLONG>((size_t)2, _stack->pop<vLONG>());
			return 0;
		case lload_3:
			_local->set<vLONG>((size_t)3, _stack->pop<vLONG>());
			return 0;

		case fload_0:
			_local->set<vFLOAT>((size_t)0, _stack->pop<vFLOAT>());
			return 0;
		case fload_1:
			_local->set<vFLOAT>((size_t)1, _stack->pop<vFLOAT>());
			return 0;
		case fload_2:
			_local->set<vFLOAT>((size_t)2, _stack->pop<vFLOAT>());
			return 0;
		case fload_3:
			_local->set<vFLOAT>((size_t)3, _stack->pop<vFLOAT>());
			return 0;

		case dload_0:
			_local->set<vDOUBLE>((size_t)0, _stack->pop<vDOUBLE>());
			return 0;
		case dload_1:
			_local->set<vDOUBLE>((size_t)1, _stack->pop<vDOUBLE>());
			return 0;
		case dload_2:
			_local->set<vDOUBLE>((size_t)2, _stack->pop<vDOUBLE>());
			return 0;
		case dload_3:
			_local->set<vDOUBLE>((size_t)3, _stack->pop<vDOUBLE>());
			return 0;

		// Store
		case astore:
			op[0].b = read<vBYTE>(ip + 1);
			_local->set<vREF>((size_t)op[0].b, _stack->pop<vREF>());
			return 1;
		case istore:
			op[0].b = read<vBYTE>(ip + 1);
			_local->set<vINT>((size_t)op[0].b, _stack->pop<vINT>());
			return 1;
		case lstore:
			op[0].b = read<vBYTE>(ip + 1);
			_local->set<vLONG>((size_t)op[0].b, _stack->pop<vLONG>());
			return 1;
		case fstore:
			op[0].b = read<vBYTE>(ip + 1);
			_local->set<vFLOAT>((size_t)op[0].b, _stack->pop<vFLOAT>());
			return 1;
		case dstore:
			op[0].b = read<vBYTE>(ip + 1);
			_local->set<vDOUBLE>((size_t)op[0].b, _stack->pop<vDOUBLE>());
			return 1;

		case astore_0:
			_local->set<vREF>((size_t)0, _stack->pop<vREF>());
			return 0;
		case astore_1:
			_local->set<vREF>((size_t)1, _stack->pop<vREF>());
			return 0;
		case astore_2:
			_local->set<vREF>((size_t)2, _stack->pop<vREF>());
			return 0;
		case astore_3:
			_local->set<vREF>((size_t)3, _stack->pop<vREF>());
			return 0;

		case istore_0:
			_local->set<vINT>((size_t)0, _stack->pop<vINT>());
			return 0;
		case istore_1:
			_local->set<vINT>((size_t)1, _stack->pop<vINT>());
			return 0;
		case istore_2:
			_local->set<vINT>((size_t)2, _stack->pop<vINT>());
			return 0;
		case istore_3:
			_local->set<vINT>((size_t)3, _stack->pop<vINT>());
			return 0;

		case lstore_0:
			_local->set<vLONG>((size_t)0, _stack->pop<vLONG>());
			return 0;
		case lstore_1:
			_local->set<vLONG>((size_t)1, _stack->pop<vLONG>());
			return 0;
		case lstore_2:
			_local->set<vLONG>((size_t)2, _stack->pop<vLONG>());
			return 0;
		case lstore_3:
			_local->set<vLONG>((size_t)3, _stack->pop<vLONG>());
			return 0;

		case fstore_0:
			_local->set<vFLOAT>((size_t)0, _stack->pop<vFLOAT>());
			return 0;
		case fstore_1:
			_local->set<vFLOAT>((size_t)1, _stack->pop<vFLOAT>());
			return 0;
		case fstore_2:
			_local->set<vFLOAT>((size_t)2, _stack->pop<vFLOAT>());
			return 0;
		case fstore_3:
			_local->set<vFLOAT>((size_t)3, _stack->pop<vFLOAT>());
			return 0;

		case dstore_0:
			_local->set<vDOUBLE>((size_t)0, _stack->pop<vDOUBLE>());
			return 0;
		case dstore_1:
			_local->set<vDOUBLE>((size_t)1, _stack->pop<vDOUBLE>());
			return 0;
		case dstore_2:
			_local->set<vDOUBLE>((size_t)2, _stack->pop<vDOUBLE>());
			return 0;
		case dstore_3:
			_local->set<vDOUBLE>((size_t)3, _stack->pop<vDOUBLE>());
			return 0;

		case ldc:
			op[0].b = read<vBYTE>(ip + 1);
			_stack->push<vCOMMON>(_constPool->get<vCOMMON>((size_t)op[0].b));
			return 1;
		case ldc_w:
		case ldc2_w:
			op[0].usi = readUSI(ip + 1);
			_stack->push<vCOMMON>(_constPool->get<vCOMMON>((size_t)op[0].usi));
			return 2;

		case iinc:
			op[0].b = read<vBYTE>(ip + 1);
			op[1].b = read<vBYTE>(ip + 2);
			_local->set<vINT>(op[0].b, _local->get<vINT>(op[0].b) + op[1].b);
			return 2;

		case goto_:
			op[0].usi = readUSI(ip + 1);
			return ((size_t)(op[0].si)) - 1;
		case goto_w:
			op[0].u = readUI(ip + 1);
			return ((size_t)(op[0].i)) - 1;
		case ifeq:
			op[0].usi = readUSI(ip + 1);
			op[1].i = _stack->pop<vINT>();
			if (op[1].i == 0)
				return ((size_t)(op[0].si)) - 1;
			else return 2;
		case ifne:
			op[0].usi = readUSI(ip + 1);
			op[1].i = _stack->pop<vINT>();
			if (op[1].i != 0)
				return ((size_t)(op[0].si)) - 1;
			else return 2;
		case ifnull:
			op[0].usi = readUSI(ip + 1);
			op[1].a = _stack->pop<vREF>();
			if (op[1].a == 0)
				return ((size_t)(op[0].si)) - 1;
			else return 2;
		case ifnonnull:
			op[0].usi = readUSI(ip + 1);
			op[1].a = _stack->pop<vREF>();
			if (op[1].a != 0)
				return ((size_t)(op[0].si)) - 1;
			else return 2;

		case iflt:
			op[0].usi = readUSI(ip + 1);
			op[1].i = _stack->pop<vINT>();
			if (op[1].i < 0)
				return ((size_t)(op[0].si)) - 1;
			else return 2;
		case ifgt:
			op[0].usi = readUSI(ip + 1);
			op[1].i = _stack->pop<vINT>();
			if (op[1].i > 0)
				return ((size_t)(op[0].si)) - 1;
			else return 2;
		case ifle:
			op[0].usi = readUSI(ip + 1);
			op[1].i = _stack->pop<vINT>();
			if (op[1].i <= 0)
				return ((size_t)(op[0].si)) - 1;
			else return 2;
		case ifge:
			op[0].usi = readUSI(ip + 1);
			op[1].i = _stack->pop<vINT>();
			if (op[1].i >= 0)
				return ((size_t)(op[0].si)) - 1;
			else return 2;

		case if_icmpeq:
			op[0].usi = readUSI(ip + 1);
			op[2].i = _stack->pop<vINT>();
			op[1].i = _stack->pop<vINT>();
			if (op[1].i == op[1].i)
				return ((size_t)(op[0].si)) - 1;
			else return 2;
		case if_icmpne:
			op[0].usi = readUSI(ip + 1);
			op[2].i = _stack->pop<vINT>();
			op[1].i = _stack->pop<vINT>();
			if (op[1].i != op[2].i)
				return ((size_t)(op[0].si)) - 1;
			else return 2;
		case if_icmplt:
			op[0].usi = readUSI(ip + 1);
			op[2].i = _stack->pop<vINT>();
			op[1].i = _stack->pop<vINT>();
			if (op[1].i < op[2].i)
				return ((size_t)(op[0].si)) - 1;
			else return 2;
		case if_icmpgt:
			op[0].usi = readUSI(ip + 1);
			op[2].i = _stack->pop<vINT>();
			op[1].i = _stack->pop<vINT>();
			if (op[1].i > op[2].i)
				return ((size_t)(op[0].si)) - 1;
			else return 2;
		case if_icmple:
			op[0].usi = readUSI(ip + 1);
			op[2].i = _stack->pop<vINT>();
			op[1].i = _stack->pop<vINT>();
			if (op[1].i <= op[2].i)
				return ((size_t)(op[0].si)) - 1;
			else return 2;
		case if_icmpge:
			op[0].usi = readUSI(ip + 1);
			op[2].i = _stack->pop<vINT>();
			op[1].i = _stack->pop<vINT>();
			if (op[1].i >= op[2].i)
				return ((size_t)(op[0].si)) - 1;
			else return 2;
		case if_acmpeq:
			op[0].usi = readUSI(ip + 1);
			op[2].a = _stack->pop<vREF>();
			op[1].a = _stack->pop<vREF>();
			if (op[1].a == op[2].a)
				return ((size_t)(op[0].si)) - 1;
			else return 2;
		case if_acmpne:
			op[0].usi = readUSI(ip + 1);
			op[2].a = _stack->pop<vREF>();
			op[1].a = _stack->pop<vREF>();
			if (op[1].a != op[2].a)
				return ((size_t)(op[0].si)) - 1;
			else return 2;

		case lcmp:
			op[1].l = _stack->pop<vLONG>();
			op[0].l = _stack->pop<vLONG>();
			_stack->push<vINT>(compare(op[0].l, op[0].l));
			return 0;
		case fcmpg:
		case fcmpl:
			op[1].f = _stack->pop<vFLOAT>();
			op[0].f = _stack->pop<vFLOAT>();
			_stack->push<vINT>(compare(op[0].f, op[0].f));
			return 0;
		case dcmpg:
		case dcmpl:
			op[1].d = _stack->pop<vDOUBLE>();
			op[0].d = _stack->pop<vDOUBLE>();
			_stack->push<vINT>(compare(op[0].d, op[0].d));
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
			retVal = _stack->pop<vCOMMON>();
			return 0;

		default:
			return 0;
		}
	}

	template<class T> T read(vBYTE* ip) const {
		return *(T*)ip;
	}

	vUSHORT readUSI(vBYTE* ip) const {
		vUSHORT usi = read<vBYTE>(ip);
		usi <<= 8;
		usi |= read<vBYTE>(ip + 1);
		return usi;
	}

	vUINT readUI(vBYTE* ip) const {
		vUINT ui = read<vBYTE>(ip);
		ui <<= 8; ui |= read<vBYTE>(ip + 1);
		ui <<= 8; ui |= read<vBYTE>(ip + 2);
		ui <<= 8; ui |= read<vBYTE>(ip + 3);
		return ui;
	}

	template<class T> vINT compare(T a, T b) {
		if (a == b) return 0;
		if (a > b) return 1;
		return -1;
	}
};

int main() {
	auto cpu = VMAKE(vCPU);
	auto stk = VMAKE(vStack);
	vBYTE bytecode[] = {
		iconst_m1,
		
		istore,
		0,

		iinc,
		0,
		1,

		iload,
		0,

		iconst_5,

		if_icmplt,
		0xFF,
		0xFA,

		iload,
		0,

		iload,
		0,

		imul,
		ireturn
	};
	auto frame = VMAKE(vFrame, V<vBYTE>(bytecode));
	vCOMMON retval = cpu->run(frame);
	printf("%d\n", retval.i);
}