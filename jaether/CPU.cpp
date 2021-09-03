#include "CPU.h"
#include "ObjectHelper.h"
#include <chrono>
#include <filesystem>
#include <codecvt>
#include <stack>

namespace jaether {

	vCPU::vCPU() {
		// ...
		registerNatives();
	}

	V<vClass> vCPU::load(vContext* ctx, const std::string& path) {
		auto& _classes = ctx->getClasses();
		auto it = _classes.find(path);
		if (it != _classes.end()) return V<vClass>((vClass*)it->second);
		if (!std::filesystem::exists(path + ".class")) {
			return V<vClass>::nullPtr();
		}
		V<vClass> cls = VMAKE(vClass, ctx, ctx, this, (path + ".class").c_str());
		_classes[cls.ptr(ctx)->getName(ctx)] = cls.v(ctx);

		vClass* clsPtr = cls.ptr(ctx);

		if (!clsPtr->_initialized) {
			clsPtr->_initialized = true;
			clsPtr->invoke(ctx, cls, cls, this, 0, invokestatic, "<clinit>", "()V");
		}

		return cls;
	}

	std::map<std::string, vClass*>::iterator vCPU::lazyLoad(vContext* ctx, const std::string& path) {
		auto& _classes = ctx->getClasses();
		auto it = _classes.find(path);
		if (it != _classes.end()) return it;
		load(ctx, path);
		it = _classes.find(path);
		return it;
	}

	bool vCPU::active() const {
		return _running;
	}

	void vCPU::addNative(const std::string& path, const std::string& desc, const vNATIVE& native)
	{
		_natives[path + ":" + desc] = native;
	}

	void vCPU::registerNatives() {
		addNative("java/lang/Object/<init>", "()V", [](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		});

		addNative("java/io/PrintStream/println", "(I)V", [](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vINT arg = stack->pop<vINT>(ctx);
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			printf("%d\n", arg);
		});


		addNative("java/io/PrintStream/println", "(J)V", [](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vLONG arg = stack->pop<vLONG>(ctx);
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			printf("%lld\n", arg);
		});

		addNative("java/lang/System/arraycopy", "(Ljava/lang/Object;ILjava/lang/Object;II)V", [](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vINT len = stack->pop<vINT>(ctx);
			vINT dstPos = stack->pop<vINT>(ctx);
			vOBJECTREF dst = stack->pop<vOBJECTREF>(ctx);
			vINT srcPos = stack->pop<vINT>(ctx);
			vOBJECTREF src = stack->pop<vOBJECTREF>(ctx);

			V<vNATIVEARRAY> srcArr((vNATIVEARRAY*)src.r.a);
			V<vNATIVEARRAY> dstArr((vNATIVEARRAY*)dst.r.a);

			auto unit = srcArr.ptr(ctx)->unitSize(srcArr.ptr(ctx)->type);
			vBYTE* pSrc = srcArr.ptr(ctx)->data.ptr(ctx);
			vBYTE* pDst = dstArr.ptr(ctx)->data.ptr(ctx);

			memmove(pDst + (size_t)dstPos * unit, pSrc + (size_t)srcPos * unit, (size_t)len * unit);

			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		});

		addNative("java/lang/System/registerNatives", "()V", [](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		});

		addNative("java/lang/Class/registerNatives", "()V", [](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		});

		addNative("java/lang/StringUTF16/isBigEndian", "()Z", [](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			stack->push<vBYTE>(ctx, 1);
		});

		addNative("java/lang/Class/desiredAssertionStatus", "()Z", [](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			stack->push<vBYTE>(ctx, 1);
		});

		addNative("java/io/PrintStream/println", "(Ljava/lang/String;)V", [](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vOBJECTREF arg = stack->pop<vOBJECTREF>(ctx);
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			JString str(ctx, arg);
			fprintf(stdout, "%s\n", str.str().c_str());
		});

		addNative("java/lang/Class/getPrimitiveClass", "(Ljava/lang/String;)Ljava/lang/Class;", [this](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vOBJECTREF arg = stack->pop<vOBJECTREF>(ctx);
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			JString str(ctx, arg);
			auto& classes = ctx->getClasses();
			auto it = lazyLoad(ctx, "java/lang/Class");
			if (it != classes.end()) {
				V<vClass> cls(it->second);
				V<vOBJECT> obj = VMAKE(vOBJECT, ctx, ctx, cls);
				vOBJECTREF ref; ref.r.a = (vULONG)obj.v(ctx);
				stack->push<vOBJECTREF>(ctx, ref);
			}
		});

		addNative("java/lang/System/currentTimeMillis", "()J", [](vContext* ctx, const std::string& cls, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			//std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
			//vLONG millis = (vLONG)ms.count();
			stack->push<vLONG>(ctx, ctx->ops() / 50000);
		});
	}

	std::chrono::steady_clock::time_point vCPU::getTime() const {
		return std::chrono::high_resolution_clock::now();
	}

	vOBJECTREF vCPU::createString(vContext* ctx, vClass* _class, vStack* _stack, vMemory* _constPool, vUSHORT strIndex, vUSHORT* backref) {
		V<vUTF8BODY> str = _class->toString(ctx, strIndex);
		vUINT size = (vUINT)str.ptr(ctx)->len;
		size_t utf16len = 0;
		const vBYTE* src = (const vBYTE*)str.ptr(ctx)->s.ptr(ctx);
		size_t utfCapacity = (size_t)size + 10;
		vJCHAR* arr = new vJCHAR[utfCapacity];
		memset(arr, 0, sizeof(vJCHAR) * utfCapacity);
		for (vUINT i = 0; i < size; i++) {
			vBYTE b = src[i] & 255;
			if (b >= 1 && b <= 0x7F) {	// 1 byte
				arr[utf16len++] = (b << 8);	// fix endian
			} else if ((b & 0xE0) == 0xC0) { // 2 bytes
				vUSHORT x = (vUSHORT)src[i++] & 255;
				vUSHORT y = (vUSHORT)src[i] & 255;
				vUSHORT R = (vUSHORT)(((x & 0x1f) << 6) | (y & 0x3f));
				arr[utf16len++] = (vJCHAR)((R >> 8) | (R << 8));	// fix endian
			} else if ((b & 0xE0) == 0xE0) { // 3 bytes
				vUSHORT x = (vUSHORT)src[i++] & 255;
				vUSHORT y = (vUSHORT)src[i++] & 255;
				vUSHORT z = (vUSHORT)src[i] & 255;
				vUSHORT R = (vUSHORT)(((x & 0xf) << 12) | ((y & 0x3f) << 6) | (z & 0x3f));
				arr[utf16len++] = (vJCHAR)((R >> 8) | (R << 8));	// fix endian
			}
		}
		std::wstring utf16Str(arr, arr + utf16len);
		delete[] arr;
		return createString(ctx, _stack, utf16Str, _constPool, backref);
	}

	vOBJECTREF vCPU::createString(vContext* ctx, vStack* _stack, const std::wstring& text, vMemory* _constPool, vUSHORT* backref) {
		V<vNATIVEARRAY> arr = VMAKE(vNATIVEARRAY, ctx, ctx, 5, (vUINT)(text.length())); // 5 = JCHAR
		for (size_t i = 0, j = text.length(); i < j; i++) {
			arr.ptr(ctx)->set<vJCHAR>(ctx, i, text[i]);
		}
		vOBJECTREF wchRef; wchRef.r.a = (uintptr_t)arr.v(ctx);
		auto it = lazyLoad(ctx, "java/lang/String");
		auto& _classes = ctx->getClasses();
		if (it != _classes.end()) {
			V<vClass> strCls = it->second;
			V<vOBJECT> strObj = VMAKE(vOBJECT, ctx, ctx, strCls);
			vOBJECTREF ref; ref.r.a = (vULONG)strObj.v(ctx);
			_stack->push<vOBJECTREF>(ctx, ref);
			_stack->push<vOBJECTREF>(ctx, wchRef);
			if (_constPool && backref) {
				_constPool->set<vOBJECTREF>(ctx, (size_t)*backref, ref);
			}
			auto [ok, result] = strCls.ptr(ctx)->invoke(ctx, strCls, strCls, this, _stack, invokespecial, "<init>", "([C)V");
			return ref;
		} else {
			vOBJECTREF objr; objr.r.a = (vULONG)V<vOBJECT>::nullPtr().v(ctx);
			return objr;
		}
	}

	size_t vCPU::run(vContext* ctx, const V<vFrame>& frame) {
		std::stack<V<vFrame>> frames;
		frames.push(frame);
		bool frameChanged = true;
		vStack* _stack = 0;
		vMemory* _local = 0;
		vClass* _class = 0;
		vMemory* _constPool = 0;
		vFrame* _frame = 0;
		auto& _classes = ctx->getClasses();
		vCOMMON op[8];
		size_t ops = 0;
		size_t fwd = 0;
		_running = true;
		bool unwrapCallstack = true;
		while (_running) {
			if (unwrapCallstack && frames.empty()) {
				return 0;
			}
			if (frameChanged) {
				const V<vFrame>& Frame = frames.top();
				_stack = Frame.ptr(ctx)->_stack.real(ctx);
				_local = Frame.ptr(ctx)->_local.real(ctx);
				_class = Frame.ptr(ctx)->_class.real(ctx);
				_constPool = Frame.ptr(ctx)->_class.ptr(ctx)->_constPool.real(ctx);
				_frame = Frame.ptr(ctx);
				if (!_frame->_program.isValid()) {
					return 0;
				}
				frameChanged = false;
			}
			vBYTE* ip = _frame->fetch(ctx);
			vBYTE& opcode = *ip; ops++;
			RPRINTF("Execute %s (%d)\n", Opcodes[opcode], opcode);
			switch (opcode) {
			case nop:
				fwd = 0; break;
			case iconst_0:
				_stack->push<vINT>(ctx, 0);
				fwd = 0; break;
			case iconst_1:
				_stack->push<vINT>(ctx, 1);
				fwd = 0; break;
			case iconst_2:
				_stack->push<vINT>(ctx, 2);
				fwd = 0; break;
			case iconst_3:
				_stack->push<vINT>(ctx, 3);
				fwd = 0; break;
			case iconst_4:
				_stack->push<vINT>(ctx, 4);
				fwd = 0; break;
			case iconst_5:
				_stack->push<vINT>(ctx, 5);
				fwd = 0; break;
			case iconst_m1:
				_stack->push<vINT>(ctx, -1);
				fwd = 0; break;
			case lconst_0:
				_stack->push<vLONG>(ctx, 0);
				fwd = 0; break;
			case lconst_1:
				_stack->push<vLONG>(ctx, 1);
				fwd = 0; break;
			case dconst_0:
				_stack->push<vDOUBLE>(ctx, 0.0);
				fwd = 0; break;
			case dconst_1:
				_stack->push<vDOUBLE>(ctx, 1.0);
				fwd = 0; break;
			case fconst_0:
				_stack->push<vFLOAT>(ctx, 0.0f);
				fwd = 0; break;
			case fconst_1:
				_stack->push<vFLOAT>(ctx, 1.0f);
				fwd = 0; break;
			case fconst_2:
				_stack->push<vFLOAT>(ctx, 2.0f);
				fwd = 0; break;
			case aconst_null:
				_stack->push<vREF>(ctx, vREF{ 0 });
				fwd = 0; break;

			case bipush:
				_stack->push<vLONG>(ctx, (vLONG)read<vCHAR>(ip + 1));
				fwd = 1; break;
			case sipush:
				op[0].usi = readUSI(ip + 1);
				_stack->push<vUSHORT>(ctx, op[0].usi);
				fwd = 2; break;
			case dup:
				op[0] = _stack->pop<vCOMMON>(ctx);
				_stack->push<vCOMMON>(ctx, op[0]);
				_stack->push<vCOMMON>(ctx, op[0]);
				fwd = 0; break;
			case dup_x1:
				op[1] = _stack->pop<vCOMMON>(ctx);
				op[0] = _stack->pop<vCOMMON>(ctx);
				_stack->push<vCOMMON>(ctx, op[1]);
				_stack->push<vCOMMON>(ctx, op[0]);
				_stack->push<vCOMMON>(ctx, op[1]);
				fwd = 0; break;

			case i2b:
				_stack->push<vBYTE>(ctx, (vBYTE)(_stack->pop<vUINT>(ctx) & 255));
				fwd = 0; break;
			case i2c:
				_stack->push<vJCHAR>(ctx, (vJCHAR)(_stack->pop<vUINT>(ctx) & 65535));
				fwd = 0; break;
			case i2s:
				_stack->push<vUSHORT>(ctx, (vUSHORT)(_stack->pop<vUINT>(ctx) & 65535));
				fwd = 0; break;
			case i2l:
				_stack->push<vLONG>(ctx, (vLONG)(_stack->pop<vINT>(ctx)));
				fwd = 0; break;
			case i2f:
				_stack->push<vFLOAT>(ctx, (vFLOAT)(_stack->pop<vINT>(ctx)));
				fwd = 0; break;
			case i2d:
				_stack->push<vDOUBLE>(ctx, (vDOUBLE)(_stack->pop<vINT>(ctx)));
				fwd = 0; break;

			case l2i:
				_stack->push<vINT>(ctx, (vINT)_stack->pop<vLONG>(ctx));
				fwd = 0; break;
			case l2f:
				_stack->push<vFLOAT>(ctx, (vFLOAT)_stack->pop<vLONG>(ctx));
				fwd = 0; break;
			case l2d:
				_stack->push<vDOUBLE>(ctx, (vDOUBLE)_stack->pop<vLONG>(ctx));
				fwd = 0; break;

			case f2d:
				_stack->push<vDOUBLE>(ctx, (vDOUBLE)_stack->pop<vFLOAT>(ctx));
				fwd = 0; break;
			case f2i:
				_stack->push<vINT>(ctx, (vINT)_stack->pop<vFLOAT>(ctx));
				fwd = 0; break;
			case f2l:
				_stack->push<vLONG>(ctx, (vLONG)_stack->pop<vFLOAT>(ctx));
				fwd = 0; break;

			case d2f:
				_stack->push<vFLOAT>(ctx, (vFLOAT)_stack->pop<vDOUBLE>(ctx));
				fwd = 0; break;
			case d2i:
				_stack->push<vINT>(ctx, (vINT)_stack->pop<vDOUBLE>(ctx));
				fwd = 0; break;
			case d2l:
				_stack->push<vLONG>(ctx, (vLONG)_stack->pop<vDOUBLE>(ctx));
				fwd = 0; break;

			case ineg:
				op[0].i = _stack->pop<vINT>(ctx);
				_stack->push<vINT>(ctx, -op[0].i);
				fwd = 0; break;
			case lneg:
				op[0].l = _stack->pop<vLONG>(ctx);
				_stack->push<vLONG>(ctx, -op[0].l);
				fwd = 0; break;
			case fneg:
				op[0].f = _stack->pop<vFLOAT>(ctx);
				_stack->push<vFLOAT>(ctx, -op[0].f);
				fwd = 0; break;
			case dneg:
				op[0].d = _stack->pop<vDOUBLE>(ctx);
				_stack->push<vDOUBLE>(ctx, -op[0].d);
				fwd = 0; break;

			case iadd:
				op[1].i = _stack->pop<vINT>(ctx);
				op[0].i = _stack->pop<vINT>(ctx);
				_stack->push<vINT>(ctx, op[0].i + op[1].i);
				fwd = 0; break;
			case ladd:
				op[1].l = _stack->pop<vLONG>(ctx);
				op[0].l = _stack->pop<vLONG>(ctx);
				_stack->push<vLONG>(ctx, op[0].l + op[1].l);
				fwd = 0; break;
			case fadd:
				op[1].f = _stack->pop<vFLOAT>(ctx);
				op[0].f = _stack->pop<vFLOAT>(ctx);
				_stack->push<vFLOAT>(ctx, op[0].f + op[1].f);
				fwd = 0; break;
			case dadd:
				op[1].d = _stack->pop<vDOUBLE>(ctx);
				op[0].d = _stack->pop<vDOUBLE>(ctx);
				_stack->push<vDOUBLE>(ctx, op[0].d + op[1].d);
				fwd = 0; break;

			case isub:
				op[1].i = _stack->pop<vINT>(ctx);
				op[0].i = _stack->pop<vINT>(ctx);
				_stack->push<vINT>(ctx, op[0].i - op[1].i);
				fwd = 0; break;
			case lsub:
				op[1].l = _stack->pop<vLONG>(ctx);
				op[0].l = _stack->pop<vLONG>(ctx);
				_stack->push<vLONG>(ctx, op[0].l - op[1].l);
				fwd = 0; break;
			case fsub:
				op[1].f = _stack->pop<vFLOAT>(ctx);
				op[0].f = _stack->pop<vFLOAT>(ctx);
				_stack->push<vFLOAT>(ctx, op[0].f - op[1].f);
				fwd = 0; break;
			case dsub:
				op[1].d = _stack->pop<vDOUBLE>(ctx);
				op[0].d = _stack->pop<vDOUBLE>(ctx);
				_stack->push<vDOUBLE>(ctx, op[0].d - op[1].d);
				fwd = 0; break;

			case idiv:
				op[1].i = _stack->pop<vINT>(ctx);
				op[0].i = _stack->pop<vINT>(ctx);
				_stack->push<vINT>(ctx, op[0].i / op[1].i);
				fwd = 0; break;
			case ldiv_:
				op[1].l = _stack->pop<vLONG>(ctx);
				op[0].l = _stack->pop<vLONG>(ctx);
				_stack->push<vLONG>(ctx, op[0].l / op[1].l);
				fwd = 0; break;
			case fdiv:
				op[1].f = _stack->pop<vFLOAT>(ctx);
				op[0].f = _stack->pop<vFLOAT>(ctx);
				_stack->push<vFLOAT>(ctx, op[0].f / op[1].f);
				fwd = 0; break;
			case ddiv:
				op[1].d = _stack->pop<vDOUBLE>(ctx);
				op[0].d = _stack->pop<vDOUBLE>(ctx);
				_stack->push<vDOUBLE>(ctx, op[0].d / op[1].d);
				fwd = 0; break;

			case imul:
				op[1].i = _stack->pop<vINT>(ctx);
				op[0].i = _stack->pop<vINT>(ctx);
				_stack->push<vINT>(ctx, op[0].i * op[1].i);
				fwd = 0; break;
			case lmul:
				op[1].l = _stack->pop<vLONG>(ctx);
				op[0].l = _stack->pop<vLONG>(ctx);
				_stack->push<vLONG>(ctx, op[0].l * op[1].l);
				fwd = 0; break;
			case fmul:
				op[1].f = _stack->pop<vFLOAT>(ctx);
				op[0].f = _stack->pop<vFLOAT>(ctx);
				_stack->push<vFLOAT>(ctx, op[0].f * op[1].f);
				fwd = 0; break;
			case dmul:
				op[1].d = _stack->pop<vDOUBLE>(ctx);
				op[0].d = _stack->pop<vDOUBLE>(ctx);
				_stack->push<vDOUBLE>(ctx, op[0].d * op[1].d);
				fwd = 0; break;

			case irem:
				op[1].i = _stack->pop<vINT>(ctx);
				op[0].i = _stack->pop<vINT>(ctx);
				_stack->push<vINT>(ctx, op[0].i % op[1].i);
				fwd = 0; break;
			case lrem:
				op[1].l = _stack->pop<vLONG>(ctx);
				op[0].l = _stack->pop<vLONG>(ctx);
				_stack->push<vLONG>(ctx, op[0].l % op[1].l);
				fwd = 0; break;
			case frem:
				op[1].f = _stack->pop<vFLOAT>(ctx);
				op[0].f = _stack->pop<vFLOAT>(ctx);
				_stack->push<vFLOAT>(ctx, std::fmodf(op[0].f, op[1].f));
				fwd = 0; break;
			case drem:
				op[1].d = _stack->pop<vDOUBLE>(ctx);
				op[0].d = _stack->pop<vDOUBLE>(ctx);
				_stack->push<vDOUBLE>(ctx, std::fmod(op[0].d, op[1].d));
				fwd = 0; break;

			case ior:
				op[1].i = _stack->pop<vINT>(ctx);
				op[0].i = _stack->pop<vINT>(ctx);
				_stack->push<vINT>(ctx, op[0].i | op[1].i);
				fwd = 0; break;
			case lor:
				op[1].l = _stack->pop<vLONG>(ctx);
				op[0].l = _stack->pop<vLONG>(ctx);
				_stack->push<vLONG>(ctx, op[0].l | op[1].l);
				fwd = 0; break;

			case iand:
				op[1].i = _stack->pop<vINT>(ctx);
				op[0].i = _stack->pop<vINT>(ctx);
				_stack->push<vINT>(ctx, op[0].i & op[1].i);
				fwd = 0; break;
			case land:
				op[1].l = _stack->pop<vLONG>(ctx);
				op[0].l = _stack->pop<vLONG>(ctx);
				_stack->push<vLONG>(ctx, op[0].l & op[1].l);
				fwd = 0; break;

			case ishr:
				op[1].i = _stack->pop<vINT>(ctx);
				op[0].i = _stack->pop<vINT>(ctx);
				_stack->push<vINT>(ctx, op[0].i >> op[1].i);
				fwd = 0; break;
			case iushr:
				op[1].u = _stack->pop<vUINT>(ctx);
				op[0].u = _stack->pop<vUINT>(ctx);
				_stack->push<vUINT>(ctx, op[0].u >> op[1].u);
				fwd = 0; break;
			case lshr:
				op[1].l = _stack->pop<vLONG>(ctx);
				op[0].l = _stack->pop<vLONG>(ctx);
				_stack->push<vLONG>(ctx, op[0].l >> op[1].l);
				fwd = 0; break;
			case lushr:
				op[1].ul = _stack->pop<vULONG>(ctx);
				op[0].ul = _stack->pop<vULONG>(ctx);
				_stack->push<vLONG>(ctx, op[0].ul >> op[1].ul);
				fwd = 0; break;

			case ishl:
				op[1].i = _stack->pop<vINT>(ctx);
				op[0].i = _stack->pop<vINT>(ctx);
				_stack->push<vINT>(ctx, op[0].i << op[1].i);
				fwd = 0; break;
			case lshl:
				op[1].l = _stack->pop<vLONG>(ctx);
				op[0].l = _stack->pop<vLONG>(ctx);
				_stack->push<vLONG>(ctx, op[0].l << op[1].l);
				fwd = 0; break;

			case ixor:
				op[1].i = _stack->pop<vINT>(ctx);
				op[0].i = _stack->pop<vINT>(ctx);
				_stack->push<vINT>(ctx, op[0].i ^ op[1].i);
				fwd = 0; break;
			case lxor:
				op[1].l = _stack->pop<vLONG>(ctx);
				op[0].l = _stack->pop<vLONG>(ctx);
				_stack->push<vLONG>(ctx, op[0].l ^ op[1].l);
				fwd = 0; break;

				// Memory operations
				// Load

			case aload:
				op[0].b = read<vBYTE>(ip + 1);
				_stack->push<vREF>(ctx, _local->get<vREF>(ctx, op[0].b));
				fwd = 1; break;
			case iload:
				op[0].b = read<vBYTE>(ip + 1);
				_stack->push<vINT>(ctx, _local->get<vINT>(ctx, op[0].b));
				fwd = 1; break;
			case lload:
				op[0].b = read<vBYTE>(ip + 1);
				_stack->push<vLONG>(ctx, _local->get<vLONG>(ctx, op[0].b));
				fwd = 1; break;
			case fload:
				op[0].b = read<vBYTE>(ip + 1);
				_stack->push<vFLOAT>(ctx, _local->get<vFLOAT>(ctx, op[0].b));
				fwd = 1; break;
			case dload:
				op[0].b = read<vBYTE>(ip + 1);
				_stack->push<vDOUBLE>(ctx, _local->get<vDOUBLE>(ctx, op[0].b));
				fwd = 1; break;

			case aload_0:
				_stack->push<vREF>(ctx, _local->get<vREF>(ctx, (size_t)0));
				fwd = 0; break;
			case aload_1:
				_stack->push<vREF>(ctx, _local->get<vREF>(ctx, (size_t)1));
				fwd = 0; break;
			case aload_2:
				_stack->push<vREF>(ctx, _local->get<vREF>(ctx, (size_t)2));
				fwd = 0; break;
			case aload_3:
				_stack->push<vREF>(ctx, _local->get<vREF>(ctx, (size_t)3));
				fwd = 0; break;

			case iload_0:
				_stack->push<vINT>(ctx, _local->get<vINT>(ctx, (size_t)0));
				fwd = 0; break;
			case iload_1:
				_stack->push<vINT>(ctx, _local->get<vINT>(ctx, (size_t)1));
				fwd = 0; break;
			case iload_2:
				_stack->push<vINT>(ctx, _local->get<vINT>(ctx, (size_t)2));
				fwd = 0; break;
			case iload_3:
				_stack->push<vINT>(ctx, _local->get<vINT>(ctx, (size_t)3));
				fwd = 0; break;

			case lload_0:
				_stack->push<vLONG>(ctx, _local->get<vLONG>(ctx, (size_t)0));
				fwd = 0; break;
			case lload_1:
				_stack->push<vLONG>(ctx, _local->get<vLONG>(ctx, (size_t)1));
				fwd = 0; break;
			case lload_2:
				_stack->push<vLONG>(ctx, _local->get<vLONG>(ctx, (size_t)2));
				fwd = 0; break;
			case lload_3:
				_stack->push<vLONG>(ctx, _local->get<vLONG>(ctx, (size_t)3));
				fwd = 0; break;

			case fload_0:
				_stack->push<vFLOAT>(ctx, _local->get<vFLOAT>(ctx, (size_t)0));
				fwd = 0; break;
			case fload_1:
				_stack->push<vFLOAT>(ctx, _local->get<vFLOAT>(ctx, (size_t)1));
				fwd = 0; break;
			case fload_2:
				_stack->push<vFLOAT>(ctx, _local->get<vFLOAT>(ctx, (size_t)2));
				fwd = 0; break;
			case fload_3:
				_stack->push<vFLOAT>(ctx, _local->get<vFLOAT>(ctx, (size_t)3));
				fwd = 0; break;

			case dload_0:
				_stack->push<vDOUBLE>(ctx, _local->get<vDOUBLE>(ctx, (size_t)0));
				fwd = 0; break;
			case dload_1:
				_stack->push<vDOUBLE>(ctx, _local->get<vDOUBLE>(ctx, (size_t)1));
				fwd = 0; break;
			case dload_2:
				_stack->push<vDOUBLE>(ctx, _local->get<vDOUBLE>(ctx, (size_t)2));
				fwd = 0; break;
			case dload_3:
				_stack->push<vDOUBLE>(ctx, _local->get<vDOUBLE>(ctx, (size_t)3));
				fwd = 0; break;

				// Store
			case astore:
				op[0].b = read<vBYTE>(ip + 1);
				_local->set<vREF>(ctx, (size_t)op[0].b, _stack->pop<vREF>(ctx));
				fwd = 1; break;
			case istore:
				op[0].b = read<vBYTE>(ip + 1);
				_local->set<vINT>(ctx, (size_t)op[0].b, _stack->pop<vINT>(ctx));
				fwd = 1; break;
			case lstore:
				op[0].b = read<vBYTE>(ip + 1);
				_local->set<vLONG>(ctx, (size_t)op[0].b, _stack->pop<vLONG>(ctx));
				fwd = 1; break;
			case fstore:
				op[0].b = read<vBYTE>(ip + 1);
				_local->set<vFLOAT>(ctx, (size_t)op[0].b, _stack->pop<vFLOAT>(ctx));
				fwd = 1; break;
			case dstore:
				op[0].b = read<vBYTE>(ip + 1);
				_local->set<vDOUBLE>(ctx, (size_t)op[0].b, _stack->pop<vDOUBLE>(ctx));
				fwd = 1; break;

			case astore_0:
				_local->set<vREF>(ctx, (size_t)0, _stack->pop<vREF>(ctx));
				fwd = 0; break;
			case astore_1:
				_local->set<vREF>(ctx, (size_t)1, _stack->pop<vREF>(ctx));
				fwd = 0; break;
			case astore_2:
				_local->set<vREF>(ctx, (size_t)2, _stack->pop<vREF>(ctx));
				fwd = 0; break;
			case astore_3:
				_local->set<vREF>(ctx, (size_t)3, _stack->pop<vREF>(ctx));
				fwd = 0; break;

			case istore_0:
				_local->set<vINT>(ctx, (size_t)0, _stack->pop<vINT>(ctx));
				fwd = 0; break;
			case istore_1:
				_local->set<vINT>(ctx, (size_t)1, _stack->pop<vINT>(ctx));
				fwd = 0; break;
			case istore_2:
				_local->set<vINT>(ctx, (size_t)2, _stack->pop<vINT>(ctx));
				fwd = 0; break;
			case istore_3:
				_local->set<vINT>(ctx, (size_t)3, _stack->pop<vINT>(ctx));
				fwd = 0; break;

			case lstore_0:
				_local->set<vLONG>(ctx, (size_t)0, _stack->pop<vLONG>(ctx));
				fwd = 0; break;
			case lstore_1:
				_local->set<vLONG>(ctx, (size_t)1, _stack->pop<vLONG>(ctx));
				fwd = 0; break;
			case lstore_2:
				_local->set<vLONG>(ctx, (size_t)2, _stack->pop<vLONG>(ctx));
				fwd = 0; break;
			case lstore_3:
				_local->set<vLONG>(ctx, (size_t)3, _stack->pop<vLONG>(ctx));
				fwd = 0; break;

			case fstore_0:
				_local->set<vFLOAT>(ctx, (size_t)0, _stack->pop<vFLOAT>(ctx));
				fwd = 0; break;
			case fstore_1:
				_local->set<vFLOAT>(ctx, (size_t)1, _stack->pop<vFLOAT>(ctx));
				fwd = 0; break;
			case fstore_2:
				_local->set<vFLOAT>(ctx, (size_t)2, _stack->pop<vFLOAT>(ctx));
				fwd = 0; break;
			case fstore_3:
				_local->set<vFLOAT>(ctx, (size_t)3, _stack->pop<vFLOAT>(ctx));
				fwd = 0; break;

			case dstore_0:
				_local->set<vDOUBLE>(ctx, (size_t)0, _stack->pop<vDOUBLE>(ctx));
				fwd = 0; break;
			case dstore_1:
				_local->set<vDOUBLE>(ctx, (size_t)1, _stack->pop<vDOUBLE>(ctx));
				fwd = 0; break;
			case dstore_2:
				_local->set<vDOUBLE>(ctx, (size_t)2, _stack->pop<vDOUBLE>(ctx));
				fwd = 0; break;
			case dstore_3:
				_local->set<vDOUBLE>(ctx, (size_t)3, _stack->pop<vDOUBLE>(ctx));
				fwd = 0; break;
			case newarray:
			{
				op[0].b = read<vBYTE>(ip + 1);
				op[1].u = _stack->pop<vUINT>(ctx);
				V<vNATIVEARRAY> arr = VMAKE(vNATIVEARRAY, ctx, ctx, op[0].b, op[1].u);
				op[2].objref.r.a = (uintptr_t)arr.v(ctx);
				_stack->push<vOBJECTREF>(ctx, op[2].objref);
				fwd = 1; break; 
			}
			case anewarray:
			{
				op[0].b = 1;
				op[1].u = _stack->pop<vUINT>(ctx);
				op[3].u = readUSI(ip + 1);
				V<vNATIVEARRAY> arr = VMAKE(vNATIVEARRAY, ctx, ctx, op[0].b, op[1].u);
				arr.ptr(ctx)->cls = op[3].u;
				op[2].objref.r.a = (uintptr_t)arr.v(ctx);
				_stack->push<vOBJECTREF>(ctx, op[2].objref);
				fwd = 2; break; 
			}
			case arraylength:
			{
				op[0].objref = _stack->pop<vOBJECTREF>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[0].objref.r.a);
				_stack->push<vUINT>(ctx, arr.ptr(ctx)->size);
				fwd = 0; break; 
			}

			case aastore:
			{
				op[0].a = _stack->pop<vREF>(ctx);
				op[1].u = _stack->pop<vUINT>(ctx);
				op[2].objref = _stack->pop<vOBJECTREF>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				arr.ptr(ctx)->set(ctx, (size_t)op[1].u, op[0].a);
				fwd = 0; break; 
			}
			case bastore:
			{
				op[0].b = _stack->pop<vBYTE>(ctx);
				op[1].u = _stack->pop<vUINT>(ctx);
				op[2].objref = _stack->pop<vOBJECTREF>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				arr.ptr(ctx)->set(ctx, (size_t)op[1].u, op[0].b);
				fwd = 0; break; 
			}
			case sastore:
			{
				op[0].si = _stack->pop<vSHORT>(ctx);
				op[1].u = _stack->pop<vUINT>(ctx);
				op[2].objref = _stack->pop<vOBJECTREF>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				arr.ptr(ctx)->set(ctx, (size_t)op[1].u, op[0].si);
				fwd = 0; break; 
			}
			case iastore:
			{
				op[0].i = _stack->pop<vINT>(ctx);
				op[1].u = _stack->pop<vUINT>(ctx);
				op[2].objref = _stack->pop<vOBJECTREF>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				arr.ptr(ctx)->set(ctx, (size_t)op[1].u, op[0].i);
				fwd = 0; break; 
			}
			case lastore:
			{
				op[0].l = _stack->pop<vLONG>(ctx);
				op[1].u = _stack->pop<vUINT>(ctx);
				op[2].objref = _stack->pop<vOBJECTREF>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				arr.ptr(ctx)->set(ctx, (size_t)op[1].u, op[0].l);
				fwd = 0; break; 
			}
			case castore:
			{
				op[0].jc = _stack->pop<vJCHAR>(ctx);
				op[1].u = _stack->pop<vUINT>(ctx);
				op[2].objref = _stack->pop<vOBJECTREF>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				arr.ptr(ctx)->set(ctx, (size_t)op[1].u, op[0].jc);
				fwd = 0; break; 
			}
			case fastore:
			{
				op[0].f = _stack->pop<vFLOAT>(ctx);
				op[1].u = _stack->pop<vUINT>(ctx);
				op[2].objref = _stack->pop<vOBJECTREF>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				arr.ptr(ctx)->set(ctx, (size_t)op[1].u, op[0].f);
				fwd = 0; break; 
			}
			case dastore:
			{
				op[0].d = _stack->pop<vDOUBLE>(ctx);
				op[1].u = _stack->pop<vUINT>(ctx);
				op[2].objref = _stack->pop<vOBJECTREF>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				arr.ptr(ctx)->set(ctx, (size_t)op[1].u, op[0].d);
				fwd = 0; break; 
			}
			// Array load
			case aaload:
			{
				op[1].u = _stack->pop<vUINT>(ctx);
				op[2].objref = _stack->pop<vOBJECTREF>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				_stack->push(ctx, arr.ptr(ctx)->get<vREF>(ctx, (size_t)op[1].u));
				fwd = 0; break; 
			}
			case baload:
			{
				op[1].u = _stack->pop<vUINT>(ctx);
				op[2].objref = _stack->pop<vOBJECTREF>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				_stack->push(ctx, arr.ptr(ctx)->get<vBYTE>(ctx, (size_t)op[1].u));
				fwd = 0; break; 
			}
			case saload:
			{
				op[1].u = _stack->pop<vUINT>(ctx);
				op[2].objref = _stack->pop<vOBJECTREF>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				_stack->push(ctx, arr.ptr(ctx)->get<vSHORT>(ctx, (size_t)op[1].u)); 
				fwd = 0; break; 
			}
			case iaload:
			{
				op[1].u = _stack->pop<vUINT>(ctx);
				op[2].objref = _stack->pop<vOBJECTREF>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				_stack->push(ctx, arr.ptr(ctx)->get<vINT>(ctx, (size_t)op[1].u));
				fwd = 0; break; 
			}
			case laload:
			{
				op[1].u = _stack->pop<vUINT>(ctx);
				op[2].objref = _stack->pop<vOBJECTREF>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				_stack->push(ctx, arr.ptr(ctx)->get<vLONG>(ctx, (size_t)op[1].u));
				fwd = 0; break; 
			}
			case caload:
			{
				op[1].u = _stack->pop<vUINT>(ctx);
				op[2].objref = _stack->pop<vOBJECTREF>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				_stack->push(ctx, arr.ptr(ctx)->get<vJCHAR>(ctx, (size_t)op[1].u));
				fwd = 0; break; 
			}
			case faload:
			{
				op[0].f = _stack->pop<vFLOAT>(ctx);
				op[1].u = _stack->pop<vUINT>(ctx);
				op[2].objref = _stack->pop<vOBJECTREF>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				_stack->push(ctx, arr.ptr(ctx)->get<vFLOAT>(ctx, (size_t)op[1].u));
				fwd = 0; break; 
			}
			case daload:
			{
				op[0].d = _stack->pop<vDOUBLE>(ctx);
				op[1].u = _stack->pop<vUINT>(ctx);
				op[2].objref = _stack->pop<vOBJECTREF>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				_stack->push(ctx, arr.ptr(ctx)->get<vDOUBLE>(ctx, (size_t)op[1].u));
				fwd = 0; break; 
			}

			case ldc:
			case ldc_w:
			case ldc2_w:
			{
				op[0].usi = opcode != ldc ? readUSI(ip + 1) : (vUSHORT)read<vBYTE>(ip + 1);
				op[1] = _constPool->get<vCOMMON>(ctx, (size_t)op[0].usi);
				if (op[1].type == vTypes::type<vSTRING>()) {
					_stack->push<vOBJECTREF>(ctx, createString(ctx, _class, _stack, _constPool, op[1].str.strIndex, &op[0].usi));
				} else {
					_stack->push<vCOMMON>(ctx, op[1]);
				}
				fwd = opcode != ldc ? 2 : 1; break;
			}
			case iinc:
				op[0].b = read<vBYTE>(ip + 1);
				op[1].c = read<vCHAR>(ip + 2);
				_local->set<vINT>(ctx, op[0].b, _local->get<vINT>(ctx, op[0].b) + op[1].c);
				fwd = 2;
				break;
			case goto_:
				op[0].usi = readUSI(ip + 1);
				fwd = ((size_t)(op[0].si)) - 1;
				break;
			case goto_w:
				op[0].u = readUI(ip + 1);
				fwd = ((size_t)(op[0].i)) - 1;
				break;
			case ifeq:
				op[0].usi = readUSI(ip + 1);
				op[1].i = _stack->pop<vINT>(ctx);
				if (op[1].i == 0)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;
			case ifne:
				op[0].usi = readUSI(ip + 1);
				op[1].i = _stack->pop<vINT>(ctx);
				if (op[1].i != 0)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2;
				break;
			case ifnull:
				op[0].usi = readUSI(ip + 1);
				op[1].a = _stack->pop<vREF>(ctx);
				if (op[1].a.a == 0)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2;
				break;
			case ifnonnull:
				op[0].usi = readUSI(ip + 1);
				op[1].a = _stack->pop<vREF>(ctx);
				if (op[1].a.a != 0)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2;
				break;
			case iflt:
				op[0].usi = readUSI(ip + 1);
				op[1].i = _stack->pop<vINT>(ctx);
				if (op[1].i < 0)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;
			case ifgt:
				op[0].usi = readUSI(ip + 1);
				op[1].i = _stack->pop<vINT>(ctx);
				if (op[1].i > 0)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;
			case ifle:
				op[0].usi = readUSI(ip + 1);
				op[1].i = _stack->pop<vINT>(ctx);
				if (op[1].i <= 0)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;
			case ifge:
				op[0].usi = readUSI(ip + 1);
				op[1].i = _stack->pop<vINT>(ctx);
				if (op[1].i >= 0)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;

			case if_icmpeq:
				op[0].usi = readUSI(ip + 1);
				op[2].i = _stack->pop<vINT>(ctx);
				op[1].i = _stack->pop<vINT>(ctx);
				if (op[1].i == op[2].i)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;
			case if_icmpne:
				op[0].usi = readUSI(ip + 1);
				op[2].i = _stack->pop<vINT>(ctx);
				op[1].i = _stack->pop<vINT>(ctx);
				if (op[1].i != op[2].i)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;
			case if_icmplt:
				op[0].usi = readUSI(ip + 1);
				op[2].i = _stack->pop<vINT>(ctx);
				op[1].i = _stack->pop<vINT>(ctx);
				if (op[1].i < op[2].i)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;
			case if_icmpgt:
				op[0].usi = readUSI(ip + 1);
				op[2].i = _stack->pop<vINT>(ctx);
				op[1].i = _stack->pop<vINT>(ctx);
				if (op[1].i > op[2].i)
					fwd = ((size_t)(op[0].si)) -1;
				else fwd = 2; break;
			case if_icmple:
				op[0].usi = readUSI(ip + 1);
				op[2].i = _stack->pop<vINT>(ctx);
				op[1].i = _stack->pop<vINT>(ctx);
				if (op[1].i <= op[2].i)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;
			case if_icmpge:
				op[0].usi = readUSI(ip + 1);
				op[2].i = _stack->pop<vINT>(ctx);
				op[1].i = _stack->pop<vINT>(ctx);
				if (op[1].i >= op[2].i)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;
			case if_acmpeq:
				op[0].usi = readUSI(ip + 1);
				op[2].a = _stack->pop<vREF>(ctx);
				op[1].a = _stack->pop<vREF>(ctx);
				if (op[1].a.a == op[2].a.a)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;
			case if_acmpne:
				op[0].usi = readUSI(ip + 1);
				op[2].a = _stack->pop<vREF>(ctx);
				op[1].a = _stack->pop<vREF>(ctx);
				if (op[1].a.a != op[2].a.a)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;

			case lcmp:
				op[1].l = _stack->pop<vLONG>(ctx);
				op[0].l = _stack->pop<vLONG>(ctx);
				_stack->push<vINT>(ctx, compare(op[0].l, op[0].l));
				fwd = 0; break;
			case fcmpg:
			case fcmpl:
				op[1].f = _stack->pop<vFLOAT>(ctx);
				op[0].f = _stack->pop<vFLOAT>(ctx);
				_stack->push<vINT>(ctx, compare(op[0].f, op[0].f));
				fwd = 0; break;
			case dcmpg:
			case dcmpl:
				op[1].d = _stack->pop<vDOUBLE>(ctx);
				op[0].d = _stack->pop<vDOUBLE>(ctx);
				_stack->push<vINT>(ctx, compare(op[0].d, op[0].d));
				fwd = 0; break;

			case return_:
			case ireturn:
			case lreturn:
			case dreturn:
			case freturn:
			case areturn:
			{
				if (unwrapCallstack) {
					if (_frame->_returns) {
						op[0] = _stack->pop<vCOMMON>(ctx);
						auto& oldFrame = frames.top();
						frames.pop();
						auto& newFrame = frames.top();
						newFrame.ptr(ctx)->_stack.ptr(ctx)->push<vCOMMON>(ctx, op[0]);
						oldFrame.ptr(ctx)->destroy(ctx);
						oldFrame.release(ctx);
					} else {
						frames.pop();
					}
					RPRINTF("<%s\n", Opcodes[opcode]);
					frameChanged = true;
				} else {
					_running = false;
				}
				fwd = 0; break;
			}

			case invokedynamic:
				op[0].usi = readUSI(ip + 1);
				op[1].mh = _constPool->get<vMETHODHANDLE>(ctx, (size_t)op[0].usi);
				op[2] = _constPool->get<vCOMMON>(ctx, (size_t)op[1].mh.index);
				fwd = 4;  break;
			case invokevirtual:
			case invokestatic:
			case invokeinterface:
			case invokespecial:
			{
				op[0].usi = readUSI(ip + 1);
				op[1].mr = _constPool->get<vMETHODREF>(ctx, (size_t)op[0].usi);
				std::string path = std::string((const char*)_class->toString(ctx, op[1].mr.clsIndex).ptr(ctx)->s.real(ctx));
				std::string methodName = (const char*)_class->toString(ctx, op[1].mr.nameIndex).ptr(ctx)->s.real(ctx);
				std::string desc = (const char*)_class->toString(ctx, op[1].mr.nameIndex, 1).ptr(ctx)->s.real(ctx);
				auto nit = _natives.find(path + "/" + methodName + ":" + desc);
				bool found = false;
				if (nit != _natives.end()) {
					nit->second(ctx, path, this, _stack, opcode);
					found = true;
				} else {
					auto it = lazyLoad(ctx, path);
					if (it != _classes.end()) {
						V<vClass> cls = it->second;
						vClass* clsPtr = cls.ptr(ctx);

						if (opcode == invokevirtual || opcode == invokeinterface) {
							vUINT argc = clsPtr->argsCount(desc.c_str());
							vCOMMON objr = _stack->get<vCOMMON>(ctx, argc);
							JObject obj(ctx, objr);
							V<vClass> objCls = obj.GetClass();
							if (objCls.isValid()) {
								cls = objCls;
								clsPtr = cls.ptr(ctx);
								path = clsPtr->getName(ctx);
							}
						}

						RPRINTF("[%s] %s/%s\n", Opcodes[opcode], path.c_str(), (methodName + desc).c_str());
						if (unwrapCallstack) {
							auto [methodFound, ret] = clsPtr->createFrame(ctx, cls, it->second, this, _stack, opcode, methodName, desc);
							if (methodFound) {
								frames.push(ret);
								frameChanged = true;
							}
							found = methodFound;
						} else {
							auto [methodFound, ret] = clsPtr->invoke(ctx, cls, it->second, this, _stack, opcode, methodName, desc);
							found = methodFound;
						}
					}
				}
				if (!found) {
					fprintf(stderr, "[vCPU::sub_execute/%s] Couldn't find virtual %s (%s)\n", Opcodes[opcode], (path + "/" + methodName).c_str(), desc.c_str());
					_running = false;
				}
				fwd = opcode == invokeinterface ? 4 : 2; break;
			}
			case new_:
			{
				op[0].usi = readUSI(ip + 1);
				op[1].mr = _constPool->get<vMETHODREF>(ctx, (size_t)op[0].usi);
				std::string path = std::string((const char*)_class->toString(ctx, op[1].mr.clsIndex).ptr(ctx)->s.real(ctx));
				auto it = lazyLoad(ctx, path);
				bool found = false;
				if (it != _classes.end()) {
					V<vClass> cls = it->second;
					V<vOBJECT> obj = VMAKE(vOBJECT, ctx, ctx, cls);
					vOBJECTREF ref; ref.r.a = (vULONG)obj.v(ctx);
					_stack->push<vOBJECTREF>(ctx, ref);
					found = true;
				}
				if (!found) {
					fprintf(stderr, "[vCPU::sub_execute/%s] Couldn't find class %s\n", Opcodes[opcode], path.c_str());
					_running = false;
				}
				fwd = 2; break;
			}
			case checkcast:
				fwd = 2;
				break;
			case getstatic:
			{
				op[0].usi = readUSI(ip + 1);
				op[3].mr = _constPool->get<vMETHODREF>(ctx, op[0].usi);
				std::string path = std::string((const char*)_class->toString(ctx, op[3].mr.clsIndex).ptr(ctx)->s.real(ctx));
				std::string field = std::string((const char*)_class->toString(ctx, op[3].mr.nameIndex).ptr(ctx)->s.real(ctx));
				auto it = lazyLoad(ctx, path);
				bool found = false;
				if (it != _classes.end()) {
					V<vClass> cls = it->second;
					vClass* clsPtr = cls.ptr(ctx);
					vFIELD* fld = clsPtr->getField(ctx, field.c_str());
					if (fld) {
						_stack->push<vCOMMON>(ctx, fld->value);
						found = true;
					}
				}
				if (!found) {
					vCOMMON dummy;
					memset(&dummy, 0, sizeof(vCOMMON));
					_stack->push<vCOMMON>(ctx, dummy);
				}
				fwd = 2; break;
			}
			case putstatic:
			{
				op[0].usi = readUSI(ip + 1);
				op[3].mr = _constPool->get<vMETHODREF>(ctx, op[0].usi);
				std::string path = std::string((const char*)_class->toString(ctx, op[3].mr.clsIndex).ptr(ctx)->s.real(ctx));
				std::string field = std::string((const char*)_class->toString(ctx, op[3].mr.nameIndex).ptr(ctx)->s.real(ctx));
				auto it = lazyLoad(ctx, path);
				bool found = false;
				if (it != _classes.end()) {
					V<vClass> cls = it->second;
					vClass* clsPtr = cls.ptr(ctx);
					vFIELD* fld = clsPtr->getField(ctx, field.c_str());
					if (fld) {
						fld->value = _stack->pop<vCOMMON>(ctx);
						found = true;
					}
				}
				if (!found) {
					_stack->pop<vCOMMON>(ctx);
				}
				fwd = 2; break;
			}
			case getfield:
			{
				op[0].usi = readUSI(ip + 1);
				op[1].objref = _stack->pop<vOBJECTREF>(ctx);
				op[3].mr = _constPool->get<vMETHODREF>(ctx, (size_t)op[0].usi);
				V<vOBJECT> obj((vOBJECT*)op[1].objref.r.a);
				V<vClass> cls = obj.ptr(ctx)->cls;
				const char* fieldName = (const char*)_class->toString(ctx, op[3].mr.nameIndex).ptr(ctx)->s.ptr(ctx);
				vCOMMON* value = cls.ptr(ctx)->getObjField(ctx, obj, fieldName);
				bool found = value != 0;
				if (found) {
					_stack->push<vCOMMON>(ctx, *value);
				} else {
					fprintf(stderr, "[vCPU::sub_execute/%s] Couldn't find field index %s::%s #%d, #%d\n", 
						Opcodes[opcode],
						_class->getName(ctx),
						_class->toString(ctx, op[3].mr.nameIndex).ptr(ctx)->s.ptr(ctx),
						op[3].mr.nameIndex,
						op[0].usi);
					_running = false;
				}
				fwd = 2; break;
			}
			case putfield:
			{
				op[0].usi = readUSI(ip + 1);
				op[2] = _stack->pop<vCOMMON>(ctx);
				op[1].objref = _stack->pop<vOBJECTREF>(ctx);
				op[3].mr = _constPool->get<vMETHODREF>(ctx, (size_t)op[0].usi);
				V<vOBJECT> obj((vOBJECT*)op[1].objref.r.a);
				V<vClass> cls = obj.ptr(ctx)->cls;
				const char* fieldName = (const char*)_class->toString(ctx, op[3].mr.nameIndex).ptr(ctx)->s.ptr(ctx);
				vCOMMON* value = cls.ptr(ctx)->getObjField(ctx, obj, fieldName);
				bool found = value != 0;
				if (found) {
					*value = op[2];
				} else {
					fprintf(stderr, "[vCPU::sub_execute/%s] Couldn't find field index %s::%s #%d\n",
						Opcodes[opcode],
						cls.ptr(ctx)->getName(ctx),
						cls.ptr(ctx)->toString(ctx, op[3].mr.nameIndex).ptr(ctx)->s.ptr(ctx),
						op[3].mr.nameIndex); 
					_running = false;
				}
				fwd = 2; break;
			}
			case pop:
				_stack->pop<vCOMMON>(ctx);
				fwd = 0; break;
			default:
				fprintf(stderr, "[vCPU::sub_execute] Executing undefined instruction with opcode %d (%s)\n", *ip, Opcodes[*ip]);
				frames.pop();
				frameChanged = true;
				if (unwrapCallstack) return 0;
				fwd = 0; break;
			}
			_frame->incrpc(fwd + 1);
			ctx->onInstruction();
		}
		_running = true;
		return ops;
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

}
