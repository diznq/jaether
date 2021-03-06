#include "CPU.h"
#include "ObjectHelper.h"
#include <chrono>
#include <filesystem>
#include <codecvt>
#include <stack>

namespace jaether {

	vCPU::vCPU() {
		registerNatives();
	}

	V<vClass> vCPU::load(vContext* ctx, const std::string& path, const int nesting) {
		auto& _classes = ctx->getClasses();
		auto it = _classes.find(path);
		if (it != _classes.end()) return it->second;
		if (!std::filesystem::exists(path + ".class")) {
			return V<vClass>::nullPtr();
		}
		V<vClass> cls = VMAKE(vClass, ctx, ctx, this, (path + ".class").c_str(), nesting);
		_classes[cls(ctx)->getName(ctx)] = cls.v(ctx);

		vClass* clsPtr = cls(ctx, W::T);
		DPRINTF("%*s>Initializing class %s\n", nesting, "", clsPtr->getName(ctx));

		if (!clsPtr->_initialized) {
			clsPtr->_initialized = true;
			V<vMETHOD> clinit = clsPtr->getMethod(ctx, "<clinit>", "()V");
			if (clinit) {
				clsPtr->invoke(ctx, cls, cls, this, 0, invokestatic, "<clinit>", "()V", nesting);
			} else {
				DPRINTF("%*s|Class has no <clinit> method\n", nesting, "");
			}
			if (ctx->_fullInit && !strcmp(clsPtr->getName(ctx), "java/lang/System")) {
				clsPtr->invoke(ctx, cls, 0, this, 0, invokestatic, "initPhase1", "()V");
			}
			DPRINTF("%*s<Class %s successfuly initialized\n", nesting, "", clsPtr->getName(ctx));
		}

		return cls;
	}

	V<vClass> vCPU::lazyLoad(vContext* ctx, const std::string& path, const int nesting) {
		std::string finalPath;
		for (char c : path) {
			finalPath += c == '.' ? '/' : c;
		}
		auto& _classes = ctx->getClasses();
		auto it = _classes.find(finalPath);
		if (it != _classes.end()) return it->second;
		load(ctx, finalPath, nesting);
		it = _classes.find(finalPath);
		if (it == _classes.end()) return V<vClass>::nullPtr();
		return it->second;
	}

	bool vCPU::active() const {
		return _running;
	}

	void vCPU::addNative(const std::string& path, const std::string& desc, const vNATIVE& native)
	{
		_natives[path + ":" + desc] = native;
	}

	std::chrono::steady_clock::time_point vCPU::getTime() const {
		return std::chrono::high_resolution_clock::now();
	}


	std::map<std::string, vNATIVE>::iterator vCPU::findNative(const std::string& path) {
		return _natives.find(path);
	}

	vOBJECTREF vCPU::createObject(vContext* ctx, const char* className, bool gc, const int nesting) {
		auto& classes = ctx->getClasses();
		auto cls = lazyLoad(ctx, className, nesting);
		if (cls) {
			V<vOBJECT> obj;
			if (gc) {
				obj = VMAKEGC(vOBJECT, ctx, ctx, cls);
			} else {
				obj = VMAKE(vOBJECT, ctx, ctx, cls);
			}
			return Ref(obj);
		} else {
			DPRINTF("%*sFailed to find class %s for object creation\n", nesting, "", className);
			throw std::runtime_error("class not found");
		}
		return vOBJECTREF{};
	}

	vOBJECTREF vCPU::createString(vContext* ctx, const vClass* _class, const vMemory* _constPool, vUSHORT strIndex, vUSHORT* backref, bool gc, const int nesting, const int source) {
		const bool cvtEndian = false;
		V<vUTF8BODY> str = _class->toString(ctx, strIndex);
		const vUINT size = (vUINT)str(ctx)->len;
		const size_t utfCapacity = (size_t)size + 10;
		const vBYTE* src = (const vBYTE*)str(ctx)->s(ctx);
		size_t utf16len = 0;
		vJCHAR* arr = new vJCHAR[utfCapacity];
		memset(arr, 0, sizeof(vJCHAR) * utfCapacity);
		for (vUINT i = 0; i < size; i++) {
			vBYTE b = src[i] & 255;
			//printf("B: %x\n", b);
			if (b >= 1 && b <= 0x7F) {	// 1 byte
				arr[utf16len++] = b;	// fix endian
			} else if ((b & 0xE0) == 0xC0) { // 2 bytes
				vUSHORT x = (vUSHORT)src[i++] & 255;
				vUSHORT y = (vUSHORT)src[i] & 255;
				vUSHORT R = (vUSHORT)(((x & 0x1f) << 6) | (y & 0x3f));
				arr[utf16len++] = (vJCHAR)R;
			} else if ((b & 0xE0) == 0xE0) { // 3 bytes
				vUSHORT x = (vUSHORT)src[i++] & 255;
				vUSHORT y = (vUSHORT)src[i++] & 255;
				vUSHORT z = (vUSHORT)src[i] & 255;
				vUSHORT R = (vUSHORT)(((x & 0xf) << 12) | ((y & 0x3f) << 6) | (z & 0x3f));
				arr[utf16len++] = (vJCHAR)R;
			}
		}
		std::wstring utf16Str(arr, arr + utf16len);
		delete[] arr;
		return createString(ctx, utf16Str, _constPool, backref, gc, nesting, source | 0x100);
	}

	vOBJECTREF vCPU::createString(vContext* ctx, const std::string& text, bool gc, const int nesting, const int source) {
		const bool cvtEndian = false;
		const vUINT size = (vUINT)text.length();
		const size_t utfCapacity = (size_t)size + 10;
		const vBYTE* src = (const vBYTE*)text.data();
		size_t utf16len = 0;
		vJCHAR* arr = new vJCHAR[utfCapacity];
		memset(arr, 0, sizeof(vJCHAR) * utfCapacity);
		for (vUINT i = 0; i < size; i++) {
			vBYTE b = src[i] & 255;
			vUSHORT R = (vUSHORT)0;
			if (b >= 1 && b <= 0x7F) {	// 1 byte
				R = b;	// fix endian
			} else if ((b & 0xE0) == 0xC0) { // 2 bytes
				vUSHORT x = (vUSHORT)src[i++] & 255;
				vUSHORT y = (vUSHORT)src[i] & 255;
				R = (vUSHORT)(((x & 0x1f) << 6) | (y & 0x3f));
			} else if ((b & 0xE0) == 0xE0) { // 3 bytes
				vUSHORT x = (vUSHORT)src[i++] & 255;
				vUSHORT y = (vUSHORT)src[i++] & 255;
				vUSHORT z = (vUSHORT)src[i] & 255;
				R = (vUSHORT)(((x & 0xf) << 12) | ((y & 0x3f) << 6) | (z & 0x3f));
			}
			arr[utf16len++] = (vJCHAR)R;
		}
		std::wstring utf16Str(arr, arr + utf16len);
		delete[] arr;
		return createString(ctx, utf16Str, 0, 0, gc, nesting, source | 0x200);
	}

	vOBJECTREF vCPU::createString(vContext* ctx, const std::wstring& text, const vMemory* _constPool, vUSHORT* backref, bool gc, const int nesting, const int source) {
		lazyLoad(ctx, "java/lang/String", nesting);
		JString str(ctx, text, gc, source);
		if (_constPool && backref) {
			_constPool->set<vOBJECTREF>(ctx, (size_t)*backref, str.ref());
		}
		return str.ref();
	}

	vOBJECTREF& vCPU::getJavaClass(vContext* ctx, const char* name, vOBJECTREF* classNameRef, bool gc) {
		std::string saneName = name;
		if (saneName.back() == ';') {
			saneName.pop_back();
			saneName = saneName.substr(1);
		}
		std::wstring wName;
		for (char& c : saneName) {
			wName += c == '/' ? L'.' : (wchar_t)c;
			if (c == '.') c = '/';
		}
		return getObject<vOBJECTREF>(ctx, "ldc:java/lang/Class:" + saneName, [this, classNameRef, wName, gc](vContext* ctx) -> vOBJECTREF {
			auto obj = createObject(ctx, "java/lang/Class", gc, 20);
			JObject wrap(ctx, obj);
			if (classNameRef) {
				wrap["name"].set(*classNameRef);
			} else {
				wrap["name"].set(createString(ctx, wName, 0, 0, gc, 0, 1));
			}
			wrap.x().set<vLONG>(1050);
			return obj;
		});
	}

	size_t vCPU::run(vContext* ctx, const V<vFrame>& frame, const int nesting) {
		std::stack<V<vFrame>> frames;
		frames.push(frame);
		bool frameChanged = true;
		vStack* Stack_ = 0;
		const vClass* Class_ = 0;
		const vMemory* Local_ = 0;
		const vMemory* ConstPool_ = 0;
		vFrame* Frame_ = 0;
		auto& _classes = ctx->getClasses();
		//V<vCOMMON> opBackend = VMAKEARRAY(vCOMMON, ctx, 8);
		vCOMMON op[8];// = opBackend(ctx);
		size_t ops = 0;
		size_t fwd = 0;
		_running = true;
		bool unwrapCallstack = true;
		while (_running) {
			//memset(op, 0, sizeof(vCOMMON) * 4);
			if (unwrapCallstack && frames.empty()) {
				return 0;
			}
			if (frameChanged) {
				const V<vFrame>& Frame = frames.top();
				Stack_ = Frame(ctx)->_stack(ctx, W::T);
				Local_ = Frame(ctx)->_local(ctx);
				Class_ = Frame(ctx)->_class(ctx);
				ConstPool_ = Frame(ctx)->_class(ctx)->_constPool(ctx);
				Frame_ = Frame(ctx, W::T);
				if (!Frame_->_program.code.isValid()) {
					return 0;
				}
				frameChanged = false;
			}
			size_t startIndex = Stack_->index();
			const vBYTE* ip = Frame_->fetch(ctx);
			const vBYTE& opcode = *ip; ops++;
			RPRINTF("|Execute %s (%d)\n", Opcodes[opcode], opcode);
			switch (opcode) {
			case nop:
				fwd = 0; break;
			case iconst_0:
				Stack_->push<vINT>(ctx, 0);
				fwd = 0; break;
			case iconst_1:
				Stack_->push<vINT>(ctx, 1);
				fwd = 0; break;
			case iconst_2:
				Stack_->push<vINT>(ctx, 2);
				fwd = 0; break;
			case iconst_3:
				Stack_->push<vINT>(ctx, 3);
				fwd = 0; break;
			case iconst_4:
				Stack_->push<vINT>(ctx, 4);
				fwd = 0; break;
			case iconst_5:
				Stack_->push<vINT>(ctx, 5);
				fwd = 0; break;
			case iconst_m1:
				Stack_->push<vINT>(ctx, -1);
				fwd = 0; break;
			case lconst_0:
				Stack_->push<vLONG>(ctx, 0);
				fwd = 0; break;
			case lconst_1:
				Stack_->push<vLONG>(ctx, 1);
				fwd = 0; break;
			case dconst_0:
				Stack_->push<vDOUBLE>(ctx, 0.0);
				fwd = 0; break;
			case dconst_1:
				Stack_->push<vDOUBLE>(ctx, 1.0);
				fwd = 0; break;
			case fconst_0:
				Stack_->push<vFLOAT>(ctx, 0.0f);
				fwd = 0; break;
			case fconst_1:
				Stack_->push<vFLOAT>(ctx, 1.0f);
				fwd = 0; break;
			case fconst_2:
				Stack_->push<vFLOAT>(ctx, 2.0f);
				fwd = 0; break;
			case aconst_null:
				op[0].a.a = 0;
				Stack_->push<vREF>(ctx, op[0].a);
				fwd = 0; break;
			case bipush:
				Stack_->push<vINT>(ctx, (vINT)read<vCHAR>(ip + 1));
				fwd = 1; break;
			case sipush:
				op[0].usi = readUSI(ip + 1);
				Stack_->push<vUSHORT>(ctx, op[0].usi);
				fwd = 2; break;
			case dup:
				op[0] = Stack_->pop<vCOMMON>(ctx);
				if (op[0].type == vTypes::type<vLONG>() || op[0].type == vTypes::type<vDOUBLE>())
					throw std::runtime_error("using dup with cat2 value");
				Stack_->push<vCOMMON>(ctx, op[0]);
				Stack_->push<vCOMMON>(ctx, op[0]);
				fwd = 0; break;
			case dup_x1:
				op[1] = Stack_->pop<vCOMMON>(ctx);
				op[0] = Stack_->pop<vCOMMON>(ctx);
				Stack_->push<vCOMMON>(ctx, op[1]);
				Stack_->push<vCOMMON>(ctx, op[0]);
				Stack_->push<vCOMMON>(ctx, op[1]);
				fwd = 0; break;
			case dup_x2:
			{
				op[0] = Stack_->pop<vCOMMON>(ctx);
				op[1] = Stack_->pop<vCOMMON>(ctx);
				bool val1Cat2 = op[0].type == vTypes::type<vDOUBLE>() || op[0].type == vTypes::type<vLONG>();
				bool val2Cat2 = op[1].type == vTypes::type<vDOUBLE>() || op[1].type == vTypes::type<vLONG>();
				bool val3Cat2 = false;
				if (!val1Cat2 && val2Cat2) {
					Stack_->push<vCOMMON>(ctx, op[0]);
					Stack_->push<vCOMMON>(ctx, op[1]);
					Stack_->push<vCOMMON>(ctx, op[0]);
				} else {
					op[2] = Stack_->pop<vCOMMON>(ctx);
					Stack_->push<vCOMMON>(ctx, op[0]);
					Stack_->push<vCOMMON>(ctx, op[2]);
					Stack_->push<vCOMMON>(ctx, op[1]);
					Stack_->push<vCOMMON>(ctx, op[0]);
				}
				fwd = 0; break;
			}

			case dup2:
				op[0] = Stack_->pop<vCOMMON>(ctx);
				if (op[0].type == vTypes::type<vDOUBLE>() || op[0].type == vTypes::type<vLONG>()) {
					Stack_->push<vCOMMON>(ctx, op[0]);
					Stack_->push<vCOMMON>(ctx, op[0]);
				} else {
					op[1] = Stack_->pop<vCOMMON>(ctx);
					if (op[1].type == vTypes::type<vDOUBLE>() || op[1].type == vTypes::type<vLONG>()) {
						Stack_->push<vCOMMON>(ctx, op[1]);
						Stack_->push<vCOMMON>(ctx, op[0]);
						Stack_->push<vCOMMON>(ctx, op[0]);
					} else {
						Stack_->push<vCOMMON>(ctx, op[1]);
						Stack_->push<vCOMMON>(ctx, op[0]);
						Stack_->push<vCOMMON>(ctx, op[1]);
						Stack_->push<vCOMMON>(ctx, op[0]);
					}
				}
				fwd = 0; break;

			case pop2:
				op[0] = Stack_->pop<vCOMMON>(ctx);
				if (op[0].type == vTypes::type<vDOUBLE>() || op[0].type == vTypes::type<vLONG>()) {
					//_stack->push<vCOMMON>(ctx, op[0]);
					//_stack->push<vCOMMON>(ctx, op[0]);
				} else {
					op[1] = Stack_->pop<vCOMMON>(ctx);
					if (op[1].type == vTypes::type<vDOUBLE>() || op[1].type == vTypes::type<vLONG>()) {
						Stack_->push<vCOMMON>(ctx, op[1]);
					} else {
						//_stack->push<vCOMMON>(ctx, op[1]);
					}
				}
				fwd = 0; break;
			case i2b:
				Stack_->push<vBYTE>(ctx, (vBYTE)(Stack_->pop<vUINT>(ctx) & 255));
				fwd = 0; break;
			case i2c:
				op[0].jc = (vJCHAR)(Stack_->pop<vUSHORT>(ctx) & 65535);
				Stack_->push<vJCHAR>(ctx, op[0].jc);
				fwd = 0; break;
			case i2s:
				Stack_->push<vUSHORT>(ctx, (vUSHORT)(Stack_->pop<vUINT>(ctx) & 65535));
				fwd = 0; break;
			case i2l:
				Stack_->push<vLONG>(ctx, (vLONG)(Stack_->pop<vINT>(ctx)));
				fwd = 0; break;
			case i2f:
				Stack_->push<vFLOAT>(ctx, (vFLOAT)(Stack_->pop<vINT>(ctx)));
				fwd = 0; break;
			case i2d:
				Stack_->push<vDOUBLE>(ctx, (vDOUBLE)(Stack_->pop<vINT>(ctx)));
				fwd = 0; break;

			case l2i:
				Stack_->push<vINT>(ctx, (vINT)Stack_->pop<vLONG>(ctx));
				fwd = 0; break;
			case l2f:
				Stack_->push<vFLOAT>(ctx, (vFLOAT)Stack_->pop<vLONG>(ctx));
				fwd = 0; break;
			case l2d:
				Stack_->push<vDOUBLE>(ctx, (vDOUBLE)Stack_->pop<vLONG>(ctx));
				fwd = 0; break;

			case f2d:
				Stack_->push<vDOUBLE>(ctx, (vDOUBLE)Stack_->pop<vFLOAT>(ctx));
				fwd = 0; break;
			case f2i:
				Stack_->push<vINT>(ctx, (vINT)Stack_->pop<vFLOAT>(ctx));
				fwd = 0; break;
			case f2l:
				Stack_->push<vLONG>(ctx, (vLONG)Stack_->pop<vFLOAT>(ctx));
				fwd = 0; break;

			case d2f:
				Stack_->push<vFLOAT>(ctx, (vFLOAT)Stack_->pop<vDOUBLE>(ctx));
				fwd = 0; break;
			case d2i:
				Stack_->push<vINT>(ctx, (vINT)Stack_->pop<vDOUBLE>(ctx));
				fwd = 0; break;
			case d2l:
				Stack_->push<vLONG>(ctx, (vLONG)Stack_->pop<vDOUBLE>(ctx));
				fwd = 0; break;

			case ineg:
				op[0].i = Stack_->pop<vINT>(ctx);
				Stack_->push<vINT>(ctx, -op[0].i);
				fwd = 0; break;
			case lneg:
				op[0].l = Stack_->pop<vLONG>(ctx);
				Stack_->push<vLONG>(ctx, -op[0].l);
				fwd = 0; break;
			case fneg:
				op[0].f = Stack_->pop<vFLOAT>(ctx);
				Stack_->push<vFLOAT>(ctx, -op[0].f);
				fwd = 0; break;
			case dneg:
				op[0].d = Stack_->pop<vDOUBLE>(ctx);
				Stack_->push<vDOUBLE>(ctx, -op[0].d);
				fwd = 0; break;

			case iadd:
				op[1].i = Stack_->pop<vINT>(ctx);
				op[0].i = Stack_->pop<vINT>(ctx);
				Stack_->push<vINT>(ctx, op[0].i + op[1].i);
				fwd = 0; break;
			case ladd:
				op[1].l = Stack_->pop<vLONG>(ctx);
				op[0].l = Stack_->pop<vLONG>(ctx);
				Stack_->push<vLONG>(ctx, op[0].l + op[1].l);
				fwd = 0; break;
			case fadd:
				op[1].f = Stack_->pop<vFLOAT>(ctx);
				op[0].f = Stack_->pop<vFLOAT>(ctx);
				Stack_->push<vFLOAT>(ctx, op[0].f + op[1].f);
				fwd = 0; break;
			case dadd:
				op[1].d = Stack_->pop<vDOUBLE>(ctx);
				op[0].d = Stack_->pop<vDOUBLE>(ctx);
				Stack_->push<vDOUBLE>(ctx, op[0].d + op[1].d);
				fwd = 0; break;

			case isub:
				op[1].i = Stack_->pop<vINT>(ctx);
				op[0].i = Stack_->pop<vINT>(ctx);
				Stack_->push<vINT>(ctx, op[0].i - op[1].i);
				fwd = 0; break;
			case lsub:
				op[1].l = Stack_->pop<vLONG>(ctx);
				op[0].l = Stack_->pop<vLONG>(ctx);
				Stack_->push<vLONG>(ctx, op[0].l - op[1].l);
				fwd = 0; break;
			case fsub:
				op[1].f = Stack_->pop<vFLOAT>(ctx);
				op[0].f = Stack_->pop<vFLOAT>(ctx);
				Stack_->push<vFLOAT>(ctx, op[0].f - op[1].f);
				fwd = 0; break;
			case dsub:
				op[1].d = Stack_->pop<vDOUBLE>(ctx);
				op[0].d = Stack_->pop<vDOUBLE>(ctx);
				Stack_->push<vDOUBLE>(ctx, op[0].d - op[1].d);
				fwd = 0; break;

			case idiv:
				op[1].i = Stack_->pop<vINT>(ctx);
				op[0].i = Stack_->pop<vINT>(ctx);
				Stack_->push<vINT>(ctx, op[0].i / op[1].i);
				fwd = 0; break;
			case ldiv_:
				op[1].l = Stack_->pop<vLONG>(ctx);
				op[0].l = Stack_->pop<vLONG>(ctx);
				Stack_->push<vLONG>(ctx, op[0].l / op[1].l);
				fwd = 0; break;
			case fdiv:
				op[1].f = Stack_->pop<vFLOAT>(ctx);
				op[0].f = Stack_->pop<vFLOAT>(ctx);
				Stack_->push<vFLOAT>(ctx, op[0].f / op[1].f);
				fwd = 0; break;
			case ddiv:
				op[1].d = Stack_->pop<vDOUBLE>(ctx);
				op[0].d = Stack_->pop<vDOUBLE>(ctx);
				Stack_->push<vDOUBLE>(ctx, op[0].d / op[1].d);
				fwd = 0; break;

			case imul:
				op[1].i = Stack_->pop<vINT>(ctx);
				op[0].i = Stack_->pop<vINT>(ctx);
				Stack_->push<vINT>(ctx, op[0].i * op[1].i);
				fwd = 0; break;
			case lmul:
				op[1].l = Stack_->pop<vLONG>(ctx);
				op[0].l = Stack_->pop<vLONG>(ctx);
				Stack_->push<vLONG>(ctx, op[0].l * op[1].l);
				fwd = 0; break;
			case fmul:
				op[1].f = Stack_->pop<vFLOAT>(ctx);
				op[0].f = Stack_->pop<vFLOAT>(ctx);
				Stack_->push<vFLOAT>(ctx, op[0].f * op[1].f);
				fwd = 0; break;
			case dmul:
				op[1].d = Stack_->pop<vDOUBLE>(ctx);
				op[0].d = Stack_->pop<vDOUBLE>(ctx);
				Stack_->push<vDOUBLE>(ctx, op[0].d * op[1].d);
				fwd = 0; break;

			case irem:
				op[1].i = Stack_->pop<vINT>(ctx);
				op[0].i = Stack_->pop<vINT>(ctx);
				Stack_->push<vINT>(ctx, op[0].i % op[1].i);
				fwd = 0; break;
			case lrem:
				op[1].l = Stack_->pop<vLONG>(ctx);
				op[0].l = Stack_->pop<vLONG>(ctx);
				Stack_->push<vLONG>(ctx, op[0].l % op[1].l);
				fwd = 0; break;
			case frem:
				op[1].f = Stack_->pop<vFLOAT>(ctx);
				op[0].f = Stack_->pop<vFLOAT>(ctx);
				Stack_->push<vFLOAT>(ctx, std::fmodf(op[0].f, op[1].f));
				fwd = 0; break;
			case drem:
				op[1].d = Stack_->pop<vDOUBLE>(ctx);
				op[0].d = Stack_->pop<vDOUBLE>(ctx);
				Stack_->push<vDOUBLE>(ctx, std::fmod(op[0].d, op[1].d));
				fwd = 0; break;

			case ior:
				op[1].i = Stack_->pop<vINT>(ctx);
				op[0].i = Stack_->pop<vINT>(ctx);
				Stack_->push<vINT>(ctx, op[0].i | op[1].i);
				fwd = 0; break;
			case lor:
				op[1].l = Stack_->pop<vLONG>(ctx);
				op[0].l = Stack_->pop<vLONG>(ctx);
				Stack_->push<vLONG>(ctx, op[0].l | op[1].l);
				fwd = 0; break;

			case iand:
				op[1].i = Stack_->pop<vINT>(ctx);
				op[0].i = Stack_->pop<vINT>(ctx);
				Stack_->push<vINT>(ctx, op[0].i & op[1].i);
				fwd = 0; break;
			case land:
				op[1].l = Stack_->pop<vLONG>(ctx);
				op[0].l = Stack_->pop<vLONG>(ctx);
				Stack_->push<vLONG>(ctx, op[0].l & op[1].l);
				fwd = 0; break;

			case ishr:
				op[1].i = Stack_->pop<vINT>(ctx);
				op[0].i = Stack_->pop<vINT>(ctx);
				Stack_->push<vINT>(ctx, op[0].i >> op[1].i);
				fwd = 0; break;
			case iushr:
				op[1].u = Stack_->pop<vUINT>(ctx);
				op[0].u = Stack_->pop<vUINT>(ctx);
				Stack_->push<vUINT>(ctx, op[0].u >> op[1].u);
				fwd = 0; break;
			case lshr:
				op[1].l = Stack_->pop<vLONG>(ctx);
				op[0].l = Stack_->pop<vLONG>(ctx);
				Stack_->push<vLONG>(ctx, op[0].l >> op[1].l);
				fwd = 0; break;
			case lushr:
				op[1].ul = Stack_->pop<vULONG>(ctx);
				op[0].ul = Stack_->pop<vULONG>(ctx);
				Stack_->push<vLONG>(ctx, op[0].ul >> op[1].ul);
				fwd = 0; break;

			case ishl:
				op[1].i = Stack_->pop<vINT>(ctx);
				op[0].i = Stack_->pop<vINT>(ctx);
				Stack_->push<vINT>(ctx, op[0].i << op[1].i);
				fwd = 0; break;
			case lshl:
				op[1].l = Stack_->pop<vLONG>(ctx);
				op[0].l = Stack_->pop<vLONG>(ctx);
				Stack_->push<vLONG>(ctx, op[0].l << op[1].l);
				fwd = 0; break;

			case ixor:
				op[1].i = Stack_->pop<vINT>(ctx);
				op[0].i = Stack_->pop<vINT>(ctx);
				Stack_->push<vINT>(ctx, op[0].i ^ op[1].i);
				fwd = 0; break;
			case lxor:
				op[1].l = Stack_->pop<vLONG>(ctx);
				op[0].l = Stack_->pop<vLONG>(ctx);
				Stack_->push<vLONG>(ctx, op[0].l ^ op[1].l);
				fwd = 0; break;

				// Memory operations
				// Load

			case aload:
				op[0].b = read<vBYTE>(ip + 1);
				op[1] = Local_->get<vCOMMON>(ctx, op[0].b);
				Stack_->push<vREF>(ctx, op[1].a);
				fwd = 1; break;
			case iload:
				op[0].b = read<vBYTE>(ip + 1);
				Stack_->push<vINT>(ctx, Local_->get<vINT>(ctx, op[0].b));
				fwd = 1; break;
			case lload:
				op[0].b = read<vBYTE>(ip + 1);
				Stack_->push<vLONG>(ctx, Local_->get<vLONG>(ctx, op[0].b));
				fwd = 1; break;
			case fload:
				op[0].b = read<vBYTE>(ip + 1);
				Stack_->push<vFLOAT>(ctx, Local_->get<vFLOAT>(ctx, op[0].b));
				fwd = 1; break;
			case dload:
				op[0].b = read<vBYTE>(ip + 1);
				Stack_->push<vDOUBLE>(ctx, Local_->get<vDOUBLE>(ctx, op[0].b));
				fwd = 1; break;

			case aload_0:
				Stack_->push<vREF>(ctx, Local_->get<vREF>(ctx, (size_t)0));
				fwd = 0; break;
			case aload_1:
				Stack_->push<vREF>(ctx, Local_->get<vREF>(ctx, (size_t)1));
				fwd = 0; break;
			case aload_2:
				Stack_->push<vREF>(ctx, Local_->get<vREF>(ctx, (size_t)2));
				fwd = 0; break;
			case aload_3:
				Stack_->push<vREF>(ctx, Local_->get<vREF>(ctx, (size_t)3));
				fwd = 0; break;

			case iload_0:
				Stack_->push<vINT>(ctx, Local_->get<vINT>(ctx, (size_t)0));
				fwd = 0; break;
			case iload_1:
				Stack_->push<vINT>(ctx, Local_->get<vINT>(ctx, (size_t)1));
				fwd = 0; break;
			case iload_2:
				Stack_->push<vINT>(ctx, Local_->get<vINT>(ctx, (size_t)2));
				fwd = 0; break;
			case iload_3:
				Stack_->push<vINT>(ctx, Local_->get<vINT>(ctx, (size_t)3));
				fwd = 0; break;

			case lload_0:
				Stack_->push<vLONG>(ctx, Local_->get<vLONG>(ctx, (size_t)0));
				fwd = 0; break;
			case lload_1:
				Stack_->push<vLONG>(ctx, Local_->get<vLONG>(ctx, (size_t)1));
				fwd = 0; break;
			case lload_2:
				Stack_->push<vLONG>(ctx, Local_->get<vLONG>(ctx, (size_t)2));
				fwd = 0; break;
			case lload_3:
				Stack_->push<vLONG>(ctx, Local_->get<vLONG>(ctx, (size_t)3));
				fwd = 0; break;

			case fload_0:
				Stack_->push<vFLOAT>(ctx, Local_->get<vFLOAT>(ctx, (size_t)0));
				fwd = 0; break;
			case fload_1:
				Stack_->push<vFLOAT>(ctx, Local_->get<vFLOAT>(ctx, (size_t)1));
				fwd = 0; break;
			case fload_2:
				Stack_->push<vFLOAT>(ctx, Local_->get<vFLOAT>(ctx, (size_t)2));
				fwd = 0; break;
			case fload_3:
				Stack_->push<vFLOAT>(ctx, Local_->get<vFLOAT>(ctx, (size_t)3));
				fwd = 0; break;

			case dload_0:
				Stack_->push<vDOUBLE>(ctx, Local_->get<vDOUBLE>(ctx, (size_t)0));
				fwd = 0; break;
			case dload_1:
				Stack_->push<vDOUBLE>(ctx, Local_->get<vDOUBLE>(ctx, (size_t)1));
				fwd = 0; break;
			case dload_2:
				Stack_->push<vDOUBLE>(ctx, Local_->get<vDOUBLE>(ctx, (size_t)2));
				fwd = 0; break;
			case dload_3:
				Stack_->push<vDOUBLE>(ctx, Local_->get<vDOUBLE>(ctx, (size_t)3));
				fwd = 0; break;

				// Store
			case astore:
				op[0].b = read<vBYTE>(ip + 1);
				Local_->set<vREF>(ctx, (size_t)op[0].b, Stack_->pop<vREF>(ctx));
				fwd = 1; break;
			case istore:
				op[0].b = read<vBYTE>(ip + 1);
				Local_->set<vINT>(ctx, (size_t)op[0].b, Stack_->pop<vINT>(ctx));
				fwd = 1; break;
			case lstore:
				op[0].b = read<vBYTE>(ip + 1);
				op[1].l = Stack_->pop<vLONG>(ctx);
				Local_->set<vLONG>(ctx, (size_t)op[0].b, op[1].l);
				fwd = 1; break;
			case fstore:
				op[0].b = read<vBYTE>(ip + 1);
				Local_->set<vFLOAT>(ctx, (size_t)op[0].b, Stack_->pop<vFLOAT>(ctx));
				fwd = 1; break;
			case dstore:
				op[0].b = read<vBYTE>(ip + 1);
				Local_->set<vDOUBLE>(ctx, (size_t)op[0].b, Stack_->pop<vDOUBLE>(ctx));
				fwd = 1; break;

			case astore_0:
				Local_->set<vREF>(ctx, (size_t)0, Stack_->pop<vREF>(ctx));
				fwd = 0; break;
			case astore_1:
				Local_->set<vREF>(ctx, (size_t)1, Stack_->pop<vREF>(ctx));
				fwd = 0; break;
			case astore_2:
				Local_->set<vREF>(ctx, (size_t)2, Stack_->pop<vREF>(ctx));
				fwd = 0; break;
			case astore_3:
				Local_->set<vREF>(ctx, (size_t)3, Stack_->pop<vREF>(ctx));
				fwd = 0; break;

			case istore_0:
				Local_->set<vINT>(ctx, (size_t)0, Stack_->pop<vINT>(ctx));
				fwd = 0; break;
			case istore_1:
				Local_->set<vINT>(ctx, (size_t)1, Stack_->pop<vINT>(ctx));
				fwd = 0; break;
			case istore_2:
				Local_->set<vINT>(ctx, (size_t)2, Stack_->pop<vINT>(ctx));
				fwd = 0; break;
			case istore_3:
				Local_->set<vINT>(ctx, (size_t)3, Stack_->pop<vINT>(ctx));
				fwd = 0; break;

			case lstore_0:
				Local_->set<vLONG>(ctx, (size_t)0, Stack_->pop<vLONG>(ctx));
				fwd = 0; break;
			case lstore_1:
				Local_->set<vLONG>(ctx, (size_t)1, Stack_->pop<vLONG>(ctx));
				fwd = 0; break;
			case lstore_2:
				Local_->set<vLONG>(ctx, (size_t)2, Stack_->pop<vLONG>(ctx));
				fwd = 0; break;
			case lstore_3:
				Local_->set<vLONG>(ctx, (size_t)3, Stack_->pop<vLONG>(ctx));
				fwd = 0; break;

			case fstore_0:
				Local_->set<vFLOAT>(ctx, (size_t)0, Stack_->pop<vFLOAT>(ctx));
				fwd = 0; break;
			case fstore_1:
				Local_->set<vFLOAT>(ctx, (size_t)1, Stack_->pop<vFLOAT>(ctx));
				fwd = 0; break;
			case fstore_2:
				Local_->set<vFLOAT>(ctx, (size_t)2, Stack_->pop<vFLOAT>(ctx));
				fwd = 0; break;
			case fstore_3:
				Local_->set<vFLOAT>(ctx, (size_t)3, Stack_->pop<vFLOAT>(ctx));
				fwd = 0; break;

			case dstore_0:
				Local_->set<vDOUBLE>(ctx, (size_t)0, Stack_->pop<vDOUBLE>(ctx));
				fwd = 0; break;
			case dstore_1:
				Local_->set<vDOUBLE>(ctx, (size_t)1, Stack_->pop<vDOUBLE>(ctx));
				fwd = 0; break;
			case dstore_2:
				Local_->set<vDOUBLE>(ctx, (size_t)2, Stack_->pop<vDOUBLE>(ctx));
				fwd = 0; break;
			case dstore_3:
				Local_->set<vDOUBLE>(ctx, (size_t)3, Stack_->pop<vDOUBLE>(ctx));
				fwd = 0; break;
			case newarray:
			{
				op[0].b = read<vBYTE>(ip + 1);
				op[1].u = Stack_->pop<vUINT>(ctx);
				V<vNATIVEARRAY> arr = VMAKEGC(vNATIVEARRAY, ctx, ctx, op[0].b, op[1].u);
				//printf("Create array of %d: %d at #%llu\n", op[0].b, op[1].u, (uintptr_t)arr.v());

				op[2].objref.r.a = (uintptr_t)arr.v(ctx);
				arr(ctx, W::T)->x.set<vLONG>(1005);
				Stack_->push<vOBJECTREF>(ctx, op[2].objref);
				fwd = 1; break; 
			}
			case anewarray:
			{
				op[0].b = 1;
				op[1].u = Stack_->pop<vUINT>(ctx);
				op[3].u = readUSI(ip + 1);
				V<vNATIVEARRAY> arr = VMAKEGC(vNATIVEARRAY, ctx, ctx, op[0].b, op[1].u);
				vCLASS cls = ConstPool_->get<vCLASS>(ctx, (size_t)op[3].u);
				auto className = std::string((const char*)Class_->toString(ctx, cls.clsIndex)(ctx)->s(ctx));
				arr(ctx, W::T)->cls = lazyLoad(ctx, className, nesting + (int)frames.size());
				op[2].objref.r.a = (uintptr_t)arr.v(ctx);
				arr(ctx, W::T)->x.set<vLONG>(1012);
				Stack_->push<vOBJECTREF>(ctx, op[2].objref);
				fwd = 2; break; 
			}
			case arraylength:
			{
				op[0] = Stack_->pop<vCOMMON>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[0].objref.r.a);
				if (arr(ctx)->TAG != JAETHER_ARR_TAG) {
					//ctx->getAllocator()->dump("memory.log");
					throw std::runtime_error("invalid array");
				}
				Stack_->push<vUINT>(ctx, arr(ctx)->size);
				fwd = 0; break; 
			}
			case aastore:
			{
				op[0] = Stack_->pop<vCOMMON>(ctx);
				op[2] = Stack_->pop<vCOMMON>(ctx);
				op[1] = Stack_->pop<vCOMMON>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[1].objref.r.a);
				if (arr(ctx)->TAG != JAETHER_ARR_TAG) throw std::runtime_error("invalid array");
				arr(ctx)->set(ctx, (size_t)op[2].u, op[0].a);
				fwd = 0; break; 
			}
			case bastore:
			{
				op[0].b = Stack_->pop<vBYTE>(ctx);
				op[1].u = Stack_->pop<vUINT>(ctx);
				op[2] = Stack_->pop<vCOMMON>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				if (arr(ctx)->TAG != JAETHER_ARR_TAG) throw std::runtime_error("invalid array");
				arr(ctx)->set(ctx, (size_t)op[1].u, op[0].b);
				fwd = 0; break; 
			}
			case sastore:
			{
				op[0].si = Stack_->pop<vSHORT>(ctx);
				op[1].u = Stack_->pop<vUINT>(ctx);
				op[2] = Stack_->pop<vCOMMON>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				if (arr(ctx)->TAG != JAETHER_ARR_TAG) throw std::runtime_error("invalid array");
				arr(ctx)->set(ctx, (size_t)op[1].u, op[0].si);
				fwd = 0; break; 
			}
			case iastore:
			{
				op[0].i = Stack_->pop<vINT>(ctx);
				op[1].u = Stack_->pop<vUINT>(ctx);
				op[2] = Stack_->pop<vCOMMON>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				if (arr(ctx)->TAG != JAETHER_ARR_TAG) throw std::runtime_error("invalid array");
				arr(ctx)->set(ctx, (size_t)op[1].u, op[0].i);
				fwd = 0; break; 
			}
			case lastore:
			{
				op[0].l = Stack_->pop<vLONG>(ctx);
				op[1].u = Stack_->pop<vUINT>(ctx);
				op[2] = Stack_->pop<vCOMMON>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				if (arr(ctx)->TAG != JAETHER_ARR_TAG) throw std::runtime_error("invalid array");
				arr(ctx)->set(ctx, (size_t)op[1].u, op[0].l);
				fwd = 0; break; 
			}
			case castore:
			{
				op[0].jc = Stack_->pop<vJCHAR>(ctx);
				op[1].u = Stack_->pop<vUINT>(ctx);
				op[2] = Stack_->pop<vCOMMON>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				if (arr(ctx)->TAG != JAETHER_ARR_TAG) throw std::runtime_error("invalid array");
				arr(ctx)->set(ctx, (size_t)op[1].u, op[0].jc);
				fwd = 0; break; 
			}
			case fastore:
			{
				op[0].f = Stack_->pop<vFLOAT>(ctx);
				op[1].u = Stack_->pop<vUINT>(ctx);
				op[2] = Stack_->pop<vCOMMON>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				if (arr(ctx)->TAG != JAETHER_ARR_TAG) throw std::runtime_error("invalid array");
				arr(ctx)->set(ctx, (size_t)op[1].u, op[0].f);
				fwd = 0; break; 
			}
			case dastore:
			{
				op[0].d = Stack_->pop<vDOUBLE>(ctx);
				op[1].u = Stack_->pop<vUINT>(ctx);
				op[2] = Stack_->pop<vCOMMON>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				if (arr(ctx)->TAG != JAETHER_ARR_TAG) throw std::runtime_error("invalid array");
				arr(ctx)->set(ctx, (size_t)op[1].u, op[0].d);
				fwd = 0; break; 
			}
			// Array load
			case aaload:
			{
				op[1].u = Stack_->pop<vUINT>(ctx);
				op[2] = Stack_->pop<vCOMMON>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				if (arr(ctx)->TAG != JAETHER_ARR_TAG) throw std::runtime_error("invalid array");
				op[3] = arr(ctx)->get<vCOMMON>(ctx, (size_t)op[1].u);
				Stack_->push<vREF>(ctx, op[3].a);
				fwd = 0; break; 
			}
			case baload:
			{
				op[1].u = Stack_->pop<vUINT>(ctx);
				op[2] = Stack_->pop<vCOMMON>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				if (arr(ctx)->TAG != JAETHER_ARR_TAG) throw std::runtime_error("invalid array");
				Stack_->push(ctx, arr(ctx)->get<vBYTE>(ctx, (size_t)op[1].u));
				fwd = 0; break; 
			}
			case saload:
			{
				op[1].u = Stack_->pop<vUINT>(ctx);
				op[2] = Stack_->pop<vCOMMON>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				if (arr(ctx)->TAG != JAETHER_ARR_TAG) throw std::runtime_error("invalid array");
				Stack_->push(ctx, arr(ctx)->get<vSHORT>(ctx, (size_t)op[1].u)); 
				fwd = 0; break; 
			}
			case iaload:
			{
				op[1].u = Stack_->pop<vUINT>(ctx);
				op[2] = Stack_->pop<vCOMMON>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				if (arr(ctx)->TAG != JAETHER_ARR_TAG) throw std::runtime_error("invalid array");
				Stack_->push(ctx, arr(ctx)->get<vINT>(ctx, (size_t)op[1].u));
				fwd = 0; break; 
			}
			case laload:
			{
				op[1].u = Stack_->pop<vUINT>(ctx);
				op[2] = Stack_->pop<vCOMMON>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				if (arr(ctx)->TAG != JAETHER_ARR_TAG) throw std::runtime_error("invalid array");
				Stack_->push(ctx, arr(ctx)->get<vLONG>(ctx, (size_t)op[1].u));
				fwd = 0; break; 
			}
			case caload:
			{
				op[1].u = Stack_->pop<vUINT>(ctx);
				op[2] = Stack_->pop<vCOMMON>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				if (arr(ctx)->TAG != JAETHER_ARR_TAG) throw std::runtime_error("invalid array");
				Stack_->push(ctx, arr(ctx)->get<vJCHAR>(ctx, (size_t)op[1].u));
				fwd = 0; break; 
			}
			case faload:
			{
				op[1].u = Stack_->pop<vUINT>(ctx);
				op[2] = Stack_->pop<vCOMMON>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				if (arr(ctx)->TAG != JAETHER_ARR_TAG) throw std::runtime_error("invalid array");
				Stack_->push(ctx, arr(ctx)->get<vFLOAT>(ctx, (size_t)op[1].u));
				fwd = 0; break; 
			}
			case daload:
			{
				op[1].u = Stack_->pop<vUINT>(ctx);
				op[2] = Stack_->pop<vCOMMON>(ctx);
				V<vNATIVEARRAY> arr((vNATIVEARRAY*)op[2].objref.r.a);
				if (arr(ctx)->TAG != JAETHER_ARR_TAG) throw std::runtime_error("invalid array");
				Stack_->push(ctx, arr(ctx)->get<vDOUBLE>(ctx, (size_t)op[1].u));
				fwd = 0; break; 
			}

			case ldc:
			case ldc_w:
			case ldc2_w:
			{
				op[0].usi = opcode != ldc ? readUSI(ip + 1) : (vUSHORT)read<vBYTE>(ip + 1);
				op[1] = ConstPool_->get<vCOMMON>(ctx, (size_t)op[0].usi);
				if (op[1].type == vTypes::type<vSTRING>()) {
					Stack_->push<vOBJECTREF>(ctx, createString(
						ctx,
						Class_,
						ConstPool_,
						op[1].str.strIndex,
						&op[0].usi,
						false,
						nesting + (int)frames.size() + 1,
						2
					));
				} else if (op[1].type == vTypes::type<vCLASS>()) {
					auto& classes = ctx->getClasses();
					auto& clsInfo = op[1].cls.clsIndex;
					std::string className = (const char*)Class_->toString(ctx, clsInfo)(ctx)->s(ctx);
					int nest = nesting + (int)frames.size();
					auto& objref = getJavaClass(ctx, className.c_str(), 0, false);
					Stack_->push<vOBJECTREF>(ctx, objref);
					RPRINTF("-Loaded class %s onto stack\n", (const char*)Class_->toString(ctx, clsInfo)(ctx)->s(ctx));
				} else {
					Stack_->push<vCOMMON>(ctx, op[1]);
				}
				fwd = opcode != ldc ? 2 : 1; break;
			}
			case iinc:
				op[0].b = read<vBYTE>(ip + 1);
				op[1].c = read<vCHAR>(ip + 2);
				Local_->set<vINT>(ctx, op[0].b, Local_->get<vINT>(ctx, op[0].b) + op[1].c);
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
				op[1].i = Stack_->pop<vINT>(ctx);
				if (op[1].i == 0)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;
			case ifne:
				op[0].usi = readUSI(ip + 1);
				op[1].i = Stack_->pop<vINT>(ctx);
				if (op[1].i != 0)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2;
				break;
			case ifnull:
				op[0].usi = readUSI(ip + 1);
				op[1] = Stack_->pop<vCOMMON>(ctx);
				if (!op[1].a.a)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2;
				break;
			case ifnonnull:
				op[0].usi = readUSI(ip + 1);
				op[1] = Stack_->pop<vCOMMON>(ctx);
				if (op[1].a.a)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2;
				break;
			case iflt:
				op[0].usi = readUSI(ip + 1);
				op[1].i = Stack_->pop<vINT>(ctx);
				if (op[1].i < 0)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;
			case ifgt:
				op[0].usi = readUSI(ip + 1);
				op[1].i = Stack_->pop<vINT>(ctx);
				if (op[1].i > 0)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;
			case ifle:
				op[0].usi = readUSI(ip + 1);
				op[1].i = Stack_->pop<vINT>(ctx);
				if (op[1].i <= 0)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;
			case ifge:
				op[0].usi = readUSI(ip + 1);
				op[1].i = Stack_->pop<vINT>(ctx);
				if (op[1].i >= 0)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;

			case if_icmpeq:
				op[0].usi = readUSI(ip + 1);
				op[2].i = Stack_->pop<vINT>(ctx);
				op[1].i = Stack_->pop<vINT>(ctx);
				if (op[1].i == op[2].i)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;
			case if_icmpne:
				op[0].usi = readUSI(ip + 1);
				op[2].i = Stack_->pop<vINT>(ctx);
				op[1].i = Stack_->pop<vINT>(ctx);
				if (op[1].i != op[2].i)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;
			case if_icmplt:
				op[0].usi = readUSI(ip + 1);
				op[2].i = Stack_->pop<vINT>(ctx);
				op[1].i = Stack_->pop<vINT>(ctx);
				if (op[1].i < op[2].i)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;
			case if_icmpgt:
				op[0].usi = readUSI(ip + 1);
				op[2].i = Stack_->pop<vINT>(ctx);
				op[1].i = Stack_->pop<vINT>(ctx);
				if (op[1].i > op[2].i)
					fwd = ((size_t)(op[0].si)) -1;
				else fwd = 2; break;
			case if_icmple:
				op[0].usi = readUSI(ip + 1);
				op[2].i = Stack_->pop<vINT>(ctx);
				op[1].i = Stack_->pop<vINT>(ctx);
				if (op[1].i <= op[2].i)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;
			case if_icmpge:
				op[0].usi = readUSI(ip + 1);
				op[2].i = Stack_->pop<vINT>(ctx);
				op[1].i = Stack_->pop<vINT>(ctx);
				if (op[1].i >= op[2].i)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;
			case if_acmpeq:
				op[0].usi = readUSI(ip + 1);
				op[2] = Stack_->pop<vCOMMON>(ctx);
				op[1] = Stack_->pop<vCOMMON>(ctx);
				if (op[1].a.a == op[2].a.a)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;
			case if_acmpne:
				op[0].usi = readUSI(ip + 1);
				op[2] = Stack_->pop<vCOMMON>(ctx);
				op[1] = Stack_->pop<vCOMMON>(ctx);
				if (op[1].a.a != op[2].a.a)
					fwd = ((size_t)(op[0].si)) - 1;
				else fwd = 2; break;

			case lcmp:
				op[1].l = Stack_->pop<vLONG>(ctx);
				op[0].l = Stack_->pop<vLONG>(ctx);
				Stack_->push<vINT>(ctx, compare(op[0].l, op[1].l));
				fwd = 0; break;
			case fcmpg:
			case fcmpl:
				op[1].f = Stack_->pop<vFLOAT>(ctx);
				op[0].f = Stack_->pop<vFLOAT>(ctx);
				Stack_->push<vINT>(ctx, compare(op[0].f, op[1].f));
				fwd = 0; break;
			case dcmpg:
			case dcmpl:
				op[1].d = Stack_->pop<vDOUBLE>(ctx);
				op[0].d = Stack_->pop<vDOUBLE>(ctx);
				Stack_->push<vINT>(ctx, compare(op[0].d, op[1].d));
				fwd = 0; break;

			case return_:
			case ireturn:
			case lreturn:
			case dreturn:
			case freturn:
			case areturn:
			{

				if (unwrapCallstack) {
					auto& oldFrame = frames.top(); frames.pop();
					if (Frame_->_returns) {
						op[0] = Stack_->pop<vCOMMON>(ctx); 
						auto& newFrame = frames.top();
						newFrame(ctx)->_stack(ctx, W::T)->push<vCOMMON>(ctx, op[0]);
					}
					oldFrame(ctx, W::T)->destroy(ctx);
					oldFrame.release(ctx);
					RPRINTF("<%s: %llu\n", Opcodes[opcode], op[0].ul);
					frameChanged = true;
				} else {
					_running = false;
				}
				fwd = 0; break;
			}

			case athrow:
			{
				std::vector<std::string> stackTrace;
				op[0] = Stack_->pop<vCOMMON>(ctx);
				JObject ex(ctx, op[0]);
				stackTrace.push_back("Stack trace: Uncaught exception: " + std::string(ex.getClass()(ctx)->getName(ctx)));
				bool found = false;
				while (!frames.empty() && !found) {
					V<vFrame> fr = frames.top();
					vFrame* _fr = fr(ctx, W::T);
					V<vClass> klass = _fr->_class;
					vULONG pc = _fr->pc();
					vUSHORT extl = _fr->_program.exceptionTableLength;
					V<ExceptionTable> ext = _fr->_program.exceptionTable;
					for (vUSHORT i = 0; i < extl && !found; i++) {
						const ExceptionTable& entry = ext(ctx, (size_t)i);
						vUSHORT startPC = readUSI((vBYTE*)&entry.startPC);
						vUSHORT endPC = readUSI((vBYTE*)&entry.endPC);
						vUSHORT handlerPC = readUSI((vBYTE*)&entry.handlerPC);
						vUSHORT catchType = readUSI((vBYTE*)&entry.catchType);
						DPRINTF("Exception entry: %d of %d, start PC: %d, end PC: %d, current PC: %llu\n", i, extl, startPC, endPC, pc);
						if (pc >= startPC && pc <= endPC) {
							std::string nm = klass(ctx)->toStdString(ctx, catchType);
							auto tgtKls = lazyLoad(ctx, nm, nesting);
							if (tgtKls(ctx)->instanceOf(ctx, ex.getClass())) {
								DPRINTF("Exception hit: %s, handler: %d\n", tgtKls(ctx)->getName(ctx), handlerPC);
								found = true;
								fwd = -1;
								_fr->_stack(ctx, W::T)->push<vCOMMON>(ctx, op[0]);
								_fr->setpc(handlerPC);
							}
						}
					}
					stackTrace.push_back(" " + _fr->getName(ctx));
					if (!found) {
						frameChanged = true;
						frames.pop();
					}
				}
				if (!found) {
					fwd = 0;
					for (std::string& msg : stackTrace) {
						printf("%s\n", msg.c_str());
					}
					throw std::runtime_error("rt exception");
				}
				break;
			}

			case invokedynamic:
				op[0].usi = readUSI(ip + 1);
				op[1].mh = ConstPool_->get<vMETHODHANDLE>(ctx, (size_t)op[0].usi);
				op[2] = ConstPool_->get<vCOMMON>(ctx, (size_t)op[1].mh.index);
				fwd = 4;  break;
			case invokevirtual:
			case invokestatic:
			case invokeinterface:
			case invokespecial:
			{
				op[0].usi = readUSI(ip + 1);
				op[1].mr = ConstPool_->get<vMETHODREF>(ctx, (size_t)op[0].usi);
				std::string path = std::string((const char*)Class_->toString(ctx, op[1].mr.clsIndex)(ctx)->s.real(ctx));
				std::string methodName = (const char*)Class_->toString(ctx, op[1].mr.nameIndex)(ctx)->s.real(ctx);
				std::string desc = (const char*)Class_->toString(ctx, op[1].mr.nameIndex, 1)(ctx)->s.real(ctx);
				auto cls = lazyLoad(ctx, path, nesting + (int)frames.size());
				bool found = false;
				if (cls) {
					auto superClass = cls;
					const vClass* clsPtr = cls(ctx);
					vUINT argc = clsPtr->argsCount(desc.c_str());
					RPRINTF("-Pre resolve path: %s::%s, %s (args: %d)\n", path.c_str(), methodName.c_str(), desc.c_str(), argc);
					if (opcode == invokevirtual || opcode == invokeinterface) {
						vCOMMON& objr = Stack_->get<vCOMMON>(ctx, argc);
						RPRINTF("-Object reference: %016llX\n", objr.objref.r.a);
						JObject obj(ctx, objr);
						if (obj) {
							V<vClass> objCls = obj.getClass();
							if (objCls.isValid()) {
								cls = objCls;
								clsPtr = cls(ctx);
								path = clsPtr->getName(ctx);
							}
						}
					}
					auto nit = _natives.find(path + "/" + methodName + ":" + desc);
					if (nit != _natives.end()) {
						RPRINTF(">[NATIVE] %s: %s/%s\n", Opcodes[opcode], path.c_str(), (methodName + desc).c_str());
						nit->second(ctx, this, Stack_, opcode);
						found = true;
					} else {
						//RPRINTF("-Post resolve path: %s::%s, %s\n", path.c_str(), methodName.c_str(), desc.c_str());

						RPRINTF(">%s: %s/%s\n", Opcodes[opcode], path.c_str(), (methodName + desc).c_str());
						if (unwrapCallstack) {
							auto [methodFound, ret] = clsPtr->createFrame(ctx, cls, superClass, this, Stack_, opcode, methodName, desc);
							if (methodFound == MethodResolveStatus::eMRS_Found) {
								frames.push(ret);
								frameChanged = true;
							}
							found = methodFound != MethodResolveStatus::eMRS_NotFound;
						} else {
							auto [methodFound, ret] = clsPtr->invoke(ctx, cls, superClass, this, Stack_, opcode, methodName, desc);
							found = methodFound != MethodResolveStatus::eMRS_NotFound;
						}
					}
				} else {
					auto nit = _natives.find(path + "/" + methodName + ":" + desc);
					if (nit != _natives.end()) {
						RPRINTF(">[NATIVE] %s: %s/%s\n", Opcodes[opcode], path.c_str(), (methodName + desc).c_str());
						nit->second(ctx, this, Stack_, opcode);
						found = true;
					}
				}
				if (!found) {
					fprintf(stderr, "[vCPU::run/%s] Couldn't find virtual %s:%s\n", Opcodes[opcode], (path + "/" + methodName).c_str(), desc.c_str());
					fprintf(stderr, "Stack trace: \n");
					while (!frames.empty()) {
						fprintf(stderr, " %s\n", frames.top()(ctx)->getName(ctx).c_str());
						frames.pop();
					}
					throw std::runtime_error("couldn't find virtual");
					_running = false;
				}
				fwd = opcode == invokeinterface ? 4 : 2; break;
			}
			case new_:
			{
				op[0].usi = readUSI(ip + 1);
				op[1].mr = ConstPool_->get<vMETHODREF>(ctx, (size_t)op[0].usi);
				std::string path = std::string((const char*)Class_->toString(ctx, op[1].mr.clsIndex)(ctx)->s.real(ctx));
				auto cls = lazyLoad(ctx, path, nesting + (int)frames.size());
				bool found = false;
				if (cls) {
					V<vOBJECT> obj = VMAKEGC(vOBJECT, ctx, ctx, cls);
					obj(ctx, W::T)->x.set<vLONG>(1004);
					Stack_->push<vOBJECTREF>(ctx, Ref(obj));
					found = true;
				}
				if (!found) {
					fprintf(stderr, "[vCPU::run/%s] Couldn't find class %s\n", Opcodes[opcode], path.c_str());
					_running = false;
				}
				fwd = 2; break;
			}
			case instanceof:
			{
				op[0].usi = readUSI(ip + 1);
				op[1] = Stack_->pop<vCOMMON>(ctx);
				op[2].cls = ConstPool_->get<vCLASS>(ctx, (size_t)op[0].usi);
				auto& classes = ctx->getClasses();
				auto cls = lazyLoad(ctx, (const char*)Class_->toString(ctx, op[2].cls.clsIndex)(ctx)->s(ctx), nesting + (int)frames.size());
				if (cls) {
					if (op[1].objref.r.a == 0) {
						Stack_->push<vBYTE>(ctx, 0);
					} else {
						JObject obj(ctx, op[1].objref);
						bool iofr = obj.getClass()(ctx)->instanceOf(ctx, cls);
						Stack_->push<vBYTE>(ctx, iofr);
					}
				} else {
					Stack_->push<vBYTE>(ctx, 0);
				}
				fwd = 2; break;
			}
			case checkcast:
				fwd = 2;
				break;
			case getstatic:
			{
				op[0].usi = readUSI(ip + 1);
				op[3].mr = ConstPool_->get<vMETHODREF>(ctx, op[0].usi);
				std::string path = std::string((const char*)Class_->toString(ctx, op[3].mr.clsIndex)(ctx)->s.real(ctx));
				std::string field = std::string((const char*)Class_->toString(ctx, op[3].mr.nameIndex)(ctx)->s.real(ctx));
				
				auto cls = lazyLoad(ctx, path, nesting + (int)frames.size());
				bool found = false;
				
				if (cls) {
					const vClass* clsPtr = cls(ctx);
					V<vFIELD> fld = clsPtr->getField(ctx, field.c_str());
					if (fld) {
						Stack_->push<vCOMMON>(ctx, fld(ctx)->value);
						found = true;
					}
				}

				if (!found) {
					throw std::runtime_error("didn't find static field");
					vCOMMON dummy;
					memset(&dummy, 0, sizeof(vCOMMON));
					Stack_->push<vCOMMON>(ctx, dummy);
				}
				fwd = 2; break;
			}
			case putstatic:
			{
				op[0].usi = readUSI(ip + 1);
				op[3].mr = ConstPool_->get<vMETHODREF>(ctx, op[0].usi);
				std::string path = std::string((const char*)Class_->toString(ctx, op[3].mr.clsIndex)(ctx)->s.real(ctx));
				std::string field = std::string((const char*)Class_->toString(ctx, op[3].mr.nameIndex)(ctx)->s.real(ctx));
				auto cls = lazyLoad(ctx, path, nesting + (int)frames.size());
				bool found = false;
				if (cls) {
					const vClass* clsPtr = cls(ctx);
					V<vFIELD> fld = clsPtr->getField(ctx, field.c_str());
					if (fld) {
						fld(ctx, W::T)->value = Stack_->pop<vCOMMON>(ctx);
						found = true;
					}
				}
				if (!found) {
					throw std::runtime_error("didn't find static field");
					Stack_->pop<vCOMMON>(ctx);
				}
				fwd = 2; break;
			}
			case getfield:
			{
				op[0].usi = readUSI(ip + 1);
				op[1] = Stack_->pop<vCOMMON>(ctx);
				op[3].mr = ConstPool_->get<vMETHODREF>(ctx, (size_t)op[0].usi);
				V<vOBJECT> obj((vOBJECT*)op[1].objref.r.a);
				V<vClass> cls = obj(ctx)->cls;
				if (obj(ctx)->TAG != JAETHER_OBJ_TAG) throw std::runtime_error("invalid object");
				if (cls(ctx)->TAG != JAETHER_CLASS_TAG) throw std::runtime_error("invalid class");
				const char* fieldName = (const char*)Class_->toString(ctx, op[3].mr.nameIndex)(ctx)->s(ctx);
				V<vCOMMON> value = cls(ctx)->getObjField(ctx, obj, fieldName);
				bool found = value.isValid();
				if (found) {
					Stack_->push<vCOMMON>(ctx, value(ctx, (size_t)0));
				} else {
					fprintf(stderr, "[vCPU::run/%s] Couldn't find field %s::%s (index: %d)\n",
						Opcodes[opcode],
						cls(ctx)->getName(ctx),
						cls(ctx)->toString(ctx, op[3].mr.nameIndex)(ctx)->s(ctx),
						op[3].mr.nameIndex);
					//_running = false;
					throw std::runtime_error("field not found");
					Stack_->push<vCOMMON>(ctx, vCOMMON{});
				}
				fwd = 2; break;
			}
			case putfield:
			{
				op[0].usi = readUSI(ip + 1);
				op[2] = Stack_->pop<vCOMMON>(ctx);
				op[1] = Stack_->pop<vCOMMON>(ctx);
				op[3].mr = ConstPool_->get<vMETHODREF>(ctx, (size_t)op[0].usi);
				V<vOBJECT> obj((vOBJECT*)op[1].objref.r.a);
				V<vClass> cls = obj(ctx)->cls;
				if (obj(ctx)->TAG != JAETHER_OBJ_TAG) throw std::runtime_error("invalid object");
				if (cls(ctx)->TAG != JAETHER_CLASS_TAG) throw std::runtime_error("invalid class");
				const char* fieldName = (const char*)Class_->toString(ctx, op[3].mr.nameIndex)(ctx)->s(ctx);
				V<vCOMMON> value = cls(ctx)->getObjField(ctx, obj, fieldName);
				bool found = value.isValid();
				if (found) {
					*(value(ctx, W::T)) = op[2];
				} else {
					fprintf(stderr, "[vCPU::run/%s] Couldn't find field %s::%s (index: %d)\n",
						Opcodes[opcode],
						cls(ctx)->getName(ctx),
						cls(ctx)->toString(ctx, op[3].mr.nameIndex)(ctx)->s(ctx),
						op[3].mr.nameIndex); 
					_running = false;
				}
				fwd = 2; break;
			}
			case pop:
				Stack_->pop<vCOMMON>(ctx);
				fwd = 0; break;
			case monitorenter:
			case monitorexit:
				Stack_->pop<vCOMMON>(ctx);
				fwd = 0; break;
			case lookupswitch:
			{
				op[0].i = Stack_->pop<vINT>(ctx);
				op[1].ul = Frame_->pc();
				op[2].ul = (3 - (op[1].ul & 3)) & 3;
				op[3].u = readUI(ip + 1 + op[2].ul);
				op[4].u = readUI(ip + 1 + op[2].ul + 4);
				vINT& index = op[0].i;
				vINT& def = op[3].i;
				vINT& pairs = op[4].i;
				vINT off = 0;

				DPRINTF("Lookup switch, index: %d, pc: %llu, pad: %llu, def: %d, pairs: %d\n",
					index,
					op[1].ul, op[2].ul,
					op[3].u, op[4].u);

				bool found = false;

				for (size_t Pair = 0; Pair < (size_t)pairs; Pair++) {
					vINT key = readUI(ip + 1 + op[2].ul + 8 + (Pair << 3));
					off = readUI(ip + 1 + op[2].ul + 8 + (Pair << 3) + 4);
					if (key == index) {
						found = true;
						break;
					}
				}
				if (!found) {
					fwd = (vULONG)((vLONG)def - 1);
				} else {
					fwd = (vULONG)((vLONG)off - 1);
				}
				break;
			}
			case tableswitch:
			{
				op[0].i = Stack_->pop<vINT>(ctx);
				op[1].ul = Frame_->pc();
				op[2].ul = (3 - (op[1].ul & 3))&3;
				op[3].u = readUI(ip + 1 + op[2].ul);
				op[4].u = readUI(ip + 1 + op[2].ul + 4);
				op[5].u = readUI(ip + 1 + op[2].ul + 8);
				vINT& index = op[0].i;
				vINT& def = op[3].i;
				vINT& lo = op[4].i;
				vINT& hi = op[5].i;

				DPRINTF("Table switch, index: %d, pc: %llu, pad: %llu, def: %d, lo: %d, hi: %d\n", 
					index,
					op[1].ul, op[2].ul,
					op[3].i, op[4].i, op[5].i);
				if (index < lo || index > hi) {
					fwd = (vULONG)((vLONG)def - 1);
				} else {
					vINT off = index - lo;
					fwd = readUI(ip + 1 + op[2].ul + 12 + off * sizeof(vINT)) - 1;
				}

				break;
			}
			default:
				fprintf(stderr, "[vCPU::run] Executing undefined instruction with opcode %d (%s)\n", *ip, Opcodes[*ip]);
				fprintf(stderr, "Stack trace: \n");
				while (!frames.empty()) {
					fprintf(stderr, " %s\n", frames.top()(ctx)->getName(ctx).c_str());
					frames.pop();
				}
				frameChanged = true;
				throw std::runtime_error("invalid instruction");
				if (unwrapCallstack) return 0;
				fwd = 0; break;
			}
			size_t endIndex = Stack_->index();
			//printf("Op: %s, start: %lld, end: %lld\n", Opcodes[opcode], startIndex, endIndex);
			//_stack->purify(ctx, startIndex, endIndex);
			Frame_->incrpc(fwd + 1);
			ctx->onInstruction();
		}
		_running = true;
		return ops;
	}

	vUSHORT vCPU::readUSI(const vBYTE* ip) const {
		vUSHORT usi = read<vBYTE>(ip);
		usi <<= 8;
		usi |= read<vBYTE>(ip + 1);
		return usi;
	}

	vUINT vCPU::readUI(const vBYTE* ip) const {
		vUINT ui = read<vBYTE>(ip);
		ui <<= 8; ui |= read<vBYTE>(ip + 1);
		ui <<= 8; ui |= read<vBYTE>(ip + 2);
		ui <<= 8; ui |= read<vBYTE>(ip + 3);
		return ui;
	}

}
