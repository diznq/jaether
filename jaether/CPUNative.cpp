#include "CPU.h"
#include "ObjectHelper.h"
#include <fstream>
#include <chrono>
#include <filesystem>
#include <stack>
#include <cmath>

namespace jaether {
	void vCPU::registerNatives() {

		addNative("java/io/PrintStream/println", "(I)V", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vINT arg = stack->pop<vINT>(ctx);
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			printf("%d\n", arg);
		});


		addNative("java/io/PrintStream/println", "(J)V", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vLONG arg = stack->pop<vLONG>(ctx);
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			printf("%lld\n", arg);
		});

		addNative("java/lang/Thread/currentThread", "()Ljava/lang/Thread;", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			auto& currentThread = getObject<vOBJECTREF>(ctx, "java/lang/Thread/currentThread/1", [this, stack](vContext* ctx) -> vOBJECTREF {
				auto objref = createObject(ctx, "java/lang/Thread", false, 20);
				auto groupref = createObject(ctx, "java/lang/ThreadGroup", false, 20);
				JObject obj(ctx, objref);
				JObject group(ctx, groupref);
				vOBJECTREF nullref; nullref.r.a = (uintptr_t)V<vOBJECT>::nullPtr().v(ctx);
				obj["name"].set<vOBJECTREF>(createString(ctx, L"CurrentThread", 0, 0, false, 0, 6));
				obj["group"].set<vOBJECTREF>(groupref);
				obj["tid"].set<vLONG>(1LL);
				obj["priority"].set<vLONG>(1LL);
				obj.x().set<vLONG>(1000);
				group["parent"].set<vOBJECTREF>(nullref);
				group.x().set<vLONG>(1007);
				return objref;
			});
			stack->push<vOBJECTREF>(ctx, currentThread);
		});

		addNative("java/lang/System/arraycopy", "(Ljava/lang/Object;ILjava/lang/Object;II)V", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vINT len = stack->pop<vINT>(ctx);
			vINT dstPos = stack->pop<vINT>(ctx);
			vOBJECTREF dst = stack->pop<vOBJECTREF>(ctx);
			vINT srcPos = stack->pop<vINT>(ctx);
			vOBJECTREF src = stack->pop<vOBJECTREF>(ctx);

			V<vNATIVEARRAY> srcArr((vNATIVEARRAY*)src.r.a);
			V<vNATIVEARRAY> dstArr((vNATIVEARRAY*)dst.r.a);

			auto unit = srcArr(ctx)->unitSize(srcArr(ctx)->type);
			vBYTE* pSrc = srcArr(ctx)->data()(ctx);
			vBYTE* pDst = dstArr(ctx)->data()(ctx);

			memmove(pDst + (size_t)dstPos * unit, pSrc + (size_t)srcPos * unit, (size_t)len * unit);

			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		});

		addNative("java/lang/System/registerNatives", "()V", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		});

		addNative("java/lang/Class/registerNatives", "()V", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		});

		addNative("jdk/internal/misc/ScopedMemoryAccess/registerNatives", "()V", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		});

		addNative("jdk/internal/misc/Unsafe/registerNatives", "()V", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		});

		addNative("java/lang/Thread/registerNatives", "()V", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		});

		addNative("java/util/concurrent/atomic/AtomicLong/VMSupportsCS8", "()Z", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			stack->push<vBYTE>(ctx, 0);
		});

		addNative("java/lang/StringUTF16/isBigEndian", "()Z", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			stack->push<vBYTE>(ctx, 0);
		});

		addNative("java/lang/Class/desiredAssertionStatus", "()Z", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			stack->push<vBYTE>(ctx, 1);
		});

		addNative("java/lang/Float/floatToRawIntBits", "(F)I", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vFLOAT flt = stack->pop<vFLOAT>(ctx);
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			stack->push<vINT>(ctx, *(vINT*)&flt);
		});

		addNative("java/lang/Double/doubleToRawLongBits", "(D)J", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vDOUBLE flt = stack->pop<vDOUBLE>(ctx);
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			stack->push<vLONG>(ctx, *(vLONG*)&flt);
		});


		addNative("java/lang/Float/intBitsToFloat", "(I)F", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vINT flt = stack->pop<vINT>(ctx);
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			stack->push<vFLOAT>(ctx, *(vFLOAT*)&flt);
		});

		addNative("java/lang/Double/longBitsToDouble", "(J)D", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vLONG flt = stack->pop<vLONG>(ctx);
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			stack->push<vDOUBLE>(ctx, *(vDOUBLE*)&flt);
		});

		addNative("java/io/PrintStream/println", "(Ljava/lang/String;)V", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vOBJECTREF arg = stack->pop<vOBJECTREF>(ctx);
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			JString str(ctx, arg);
			fprintf(stdout, "%s\n", str.str().c_str());
		});

		addNative("java/lang/Class/getPrimitiveClass", "(Ljava/lang/String;)Ljava/lang/Class;", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vOBJECTREF arg = stack->pop<vOBJECTREF>(ctx);
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			JString str(ctx, arg);
			auto& ref = getJavaClass(ctx, str.str().c_str(), &arg);
			stack->push<vOBJECTREF>(ctx, ref);
		});

		addNative("java/lang/System/currentTimeMillis", "()J", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			stack->push<vLONG>(ctx, ctx->ops() / 50000);
		});

		addNative("java/lang/System/nanoTime", "()J", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			stack->push<vLONG>(ctx, ctx->ops() * 65527);
		});


		addNative("java/lang/System/gc", "()V", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			for (int GCC = 0; GCC < 2; GCC++) {
				printf("Freed %llu bytes\n", ctx->getAllocator()->gcCycle());
			}
		});

		addNative("jdk/internal/misc/Unsafe/arrayBaseOffset0", "(Ljava/lang/Class;)I", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			JObject classObj(ctx, stack->pop<vOBJECTREF>(ctx));
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			JString name(ctx, classObj["name"]);
			//printf("array base offset: %s, %s\n", classObj.getClass()(ctx)->getName(ctx), name.str().c_str());
			stack->push<vINT>(ctx, 0);
		});


		addNative("jdk/internal/misc/Unsafe/arrayIndexScale0", "(Ljava/lang/Class;)I", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			JObject classObj(ctx, stack->pop<vOBJECTREF>(ctx));
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			JString name(ctx, classObj["name"]);
			int scale = 0;
			std::string tmp = name.str();
			if (tmp[0] == '[') {
				switch (tmp[1]) {
				case 'Z': case 'B': scale = sizeof(vBYTE); break;
				case 'S': scale = sizeof(vSHORT); break;
				case 'I': scale = sizeof(vINT); break;
				case 'J': scale = sizeof(vLONG); break;
				case 'D': scale = sizeof(vDOUBLE); break;
				case 'F': scale = sizeof(vFLOAT); break;
				case 'L': scale = sizeof(vCOMMON); break;
				}
			}
			//printf("array index scale: %s, %s\n", classObj.getClass()(ctx)->getName(ctx), name.str().c_str());
			stack->push<vINT>(ctx, scale);
		});

		//jdk/internal/misc/Unsafe/objectFieldOffset1:(Ljava/lang/Class;Ljava/lang/String;)J
		addNative("jdk/internal/misc/Unsafe/objectFieldOffset1", "(Ljava/lang/Class;Ljava/lang/String;)J", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			JString fieldName(ctx, stack->pop<vOBJECTREF>(ctx));
			JObject classObj(ctx, stack->pop<vOBJECTREF>(ctx));
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			JString name(ctx, classObj["name"]);
			auto klass = lazyLoad(ctx, name.str(), 20);

			for (int k = 0, i=0; k < klass(ctx)->_fieldCount; k++) {
				vFIELD& fld = klass(ctx)->_fields(ctx, k);
				if (fld.access & 8) continue;
				V<vClass> fldcls(fld.cls);
				std::string currentName(
					(const char*)fldcls(ctx)->toString(ctx, fld.name)(ctx)->s(ctx)
				);
				if (currentName == fieldName.str()) {
					stack->push<vLONG>(ctx, (vLONG)i);
					return;
				}
				i++;
			}
			//printf("%s doesnt have field %s\n", saneName.c_str(), fieldName.str().c_str());
			throw std::runtime_error("couldn't find offset");
			stack->push<vLONG>(ctx, -1);
		});

		//jdk/internal/misc/Unsafe/objectFieldOffset0:(Ljava/lang/reflect/Field;)J
		addNative("jdk/internal/misc/Unsafe/objectFieldOffset0", "(Ljava/lang/reflect/Field;)J", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			JObject field(ctx, stack->pop<vOBJECTREF>(ctx));
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			JString fieldName(ctx, field["name"]);
			JObject classObj(ctx, field["clazz"]);
			JString name(ctx, classObj["name"]);
			auto klass = lazyLoad(ctx, name.str(), 20);

			for (int k = 0, i = 0; k < klass(ctx)->_fieldCount; k++) {
				vFIELD& fld = klass(ctx)->_fields(ctx, k);
				if (fld.access & 8) continue;
				V<vClass> fldcls(fld.cls);
				std::string currentName(
					(const char*)fldcls(ctx)->toString(ctx, fld.name)(ctx)->s(ctx)
				);
				if (currentName == fieldName.str()) {
					stack->push<vLONG>(ctx, (vLONG)i);
					return;
				}
				i++;
			}
			//printf("%s doesnt have field %s\n", saneName.c_str(), fieldName.str().c_str());
			throw std::runtime_error("couldn't find offset");
			stack->push<vLONG>(ctx, -1);
		});

		addNative("java/lang/Class/forName0", "(Ljava/lang/String;ZLjava/lang/ClassLoader;Ljava/lang/Class;)Ljava/lang/Class;", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vOBJECTREF classRef = stack->pop<vOBJECTREF>(ctx);
			vOBJECTREF classLoader = stack->pop<vOBJECTREF>(ctx);
			vBYTE whatever = stack->pop<vBYTE>(ctx);
			vOBJECTREF classNameRef = stack->pop<vOBJECTREF>(ctx);
			JString className(ctx, classNameRef);
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			std::string saneName;
			std::string tmp = className.str();
			for (char c : tmp) {
				if (c == '.') saneName += '/';
				else saneName += c;
			}

			auto& ref = getJavaClass(ctx, saneName.c_str(), &classNameRef);

			stack->push<vOBJECTREF>(ctx, ref);
		});

		//java/security/AccessController/getStackAccessControlContext:()Ljava/security/AccessControlContext;

		addNative("java/security/AccessController/getStackAccessControlContext", "()Ljava/security/AccessControlContext;", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			/*auto& ref = getObject<vOBJECTREF>(ctx, "java/security/AccessController/getStackAccessControlContext", [this](vContext* ctx) -> vOBJECTREF {
				auto obj = createObject(ctx, "java/security/AccessControlContext", false, 20);
				JObject wrap(ctx, obj);
				wrap.x().set<vLONG>(1011);
				return obj;
			});*/
			vOBJECTREF nullref; nullref.r.a = 0;
			stack->push<vOBJECTREF>(ctx, nullref);
		});

		addNative("java/security/AccessController/getInheritedAccessControlContext", "()Ljava/security/AccessControlContext;", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			auto& ref = getObject<vOBJECTREF>(ctx, "java/security/AccessController/getInheritedAccessControlContext", [this](vContext* ctx) -> vOBJECTREF {
				auto obj = createObject(ctx, "java/security/AccessControlContext", false, 20);
				JObject wrap(ctx, obj);
				wrap.x().set<vLONG>(1012);
				return obj;
			});
			stack->push<vOBJECTREF>(ctx, ref);
		});

		//java/lang/Thread/setPriority0:(I)V

		addNative("java/lang/Thread/setPriority0", "(I)V", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vINT priority = stack->pop<vINT>(ctx);
			if (opcode != invokestatic) {
				JObject thr(ctx, stack->pop<vCOMMON>(ctx));
				thr["priority"].set(priority);
			}
		});

		addNative("java/lang/Thread/isAlive", "()Z", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vBYTE alive = 0;
			if (opcode != invokestatic) {
				JObject thr(ctx, stack->pop<vCOMMON>(ctx));
				alive = thr.x().i >= 2000;
			}
			stack->push<vBYTE>(ctx, alive);
		});

		//java/lang/Thread/start0:()V
		addNative("java/lang/Thread/start0", "()V", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) {
				JObject thr(ctx, stack->pop<vCOMMON>(ctx));
				thr.x().i += 1001;
			}
		});

		//java/lang/Throwable/fillInStackTrace:(I)Ljava/lang/Throwable;
		addNative("java/lang/Throwable/fillInStackTrace", "(I)Ljava/lang/Throwable;", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vINT a = stack->pop<vINT>(ctx);
			if (opcode != invokestatic) {
				stack->pop<vCOMMON>(ctx);
			}
			stack->push<vOBJECTREF>(ctx, createObject(ctx, "java/lang/Throwable"));
		});

		// jdk/internal/misc/Unsafe/compareAndSetReference:(Ljava/lang/Object;JLjava/lang/Object;Ljava/lang/Object;)Z

		addNative("jdk/internal/misc/Unsafe/compareAndSetReference", "(Ljava/lang/Object;JLjava/lang/Object;Ljava/lang/Object;)Z", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vCOMMON x = stack->pop<vCOMMON>(ctx);
			vCOMMON expected = stack->pop<vCOMMON>(ctx);
			vLONG   offset = stack->pop<vLONG>(ctx);
			vCOMMON o = stack->pop<vCOMMON>(ctx);
			vCOMMON* pfld = 0;

			//printf("Compare and set reference: %lld, %lld, %lld\n", offset, expected.objref.r.a, x.objref.r.a);
			JObject tgt(ctx, o);
			JObject fobj(ctx, x);
			//printf("Fobj: %s\n", fobj.getClass()(ctx)->getName(ctx));
			pfld = &tgt[(size_t)offset];
			if (opcode != invokestatic) {
				stack->pop<vCOMMON>(ctx);
			}
			bool updated = pfld->objref.r.a == expected.objref.r.a;
			if (updated) {
				pfld->objref.r.a = x.objref.r.a;
			}
			stack->push<vBYTE>(ctx, updated);
		});

		addNative("jdk/internal/misc/Unsafe/compareAndSetLong", "(Ljava/lang/Object;JJJ)Z", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vLONG x = stack->pop<vLONG>(ctx);
			vLONG expected = stack->pop<vLONG>(ctx);
			vLONG   offset = stack->pop<vLONG>(ctx);
			vCOMMON o = stack->pop<vCOMMON>(ctx);
			vCOMMON* pfld = 0;
			JObject tgt(ctx, o);
			pfld = &tgt[(size_t)offset];
			if (opcode != invokestatic) {
				stack->pop<vCOMMON>(ctx);
			}
			bool updated = pfld && pfld->l == expected;
			/*printf(
				"Compare and set long:\n"
				" Expected: %llu\n"
				" Current: %llu\n"
				" Target: %llu\n"
				" State: %d\n", expected, pfld->l, x, updated);*/
			
			if (updated) {
				pfld->l = x;
			}
			stack->push<vBYTE>(ctx, updated);
		});

		addNative("jdk/internal/misc/Unsafe/compareAndSetInt", "(Ljava/lang/Object;JII)Z", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vINT x = stack->pop<vINT>(ctx);
			vINT expected = stack->pop<vINT>(ctx);
			vLONG   offset = stack->pop<vLONG>(ctx);
			vCOMMON o = stack->pop<vCOMMON>(ctx);
			vCOMMON* pfld = 0;
			JObject tgt(ctx, o);
			pfld = &tgt[(size_t)offset];
			if (opcode != invokestatic) {
				stack->pop<vCOMMON>(ctx);
			}
			bool updated = pfld->i == expected;
			if (updated) {
				pfld->i = x;
			}
			stack->push<vBYTE>(ctx, updated);
		});

		addNative("jdk/internal/misc/Unsafe/getReferenceVolatile", "(Ljava/lang/Object;J)Ljava/lang/Object;", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vLONG   offset = stack->pop<vLONG>(ctx);
			vCOMMON o = stack->pop<vCOMMON>(ctx);
			vCOMMON* pfld = 0;
			JObject tgt(ctx, o);
			pfld = &tgt[(size_t)(offset)];
			if (opcode != invokestatic) {
				stack->pop<vCOMMON>(ctx);
			}
			stack->push<vOBJECTREF>(ctx, pfld->objref);
		});

		addNative("jdk/internal/misc/Unsafe/putReferenceVolatile", "(Ljava/lang/Object;JLjava/lang/Object;)V", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vCOMMON target = stack->pop<vCOMMON>(ctx);
			vLONG   offset = stack->pop<vLONG>(ctx);
			vCOMMON o = stack->pop<vCOMMON>(ctx);
			vCOMMON* pfld = 0;
			JObject tgt(ctx, o);
			pfld = &tgt[(size_t)(offset)];
			if (opcode != invokestatic) {
				stack->pop<vCOMMON>(ctx);
			}
			pfld->objref = target.objref;
			//stack->push<vOBJECTREF>(ctx, pfld->objref);
		});

		addNative("jdk/internal/misc/Unsafe/getIntVolatile", "(Ljava/lang/Object;J)I", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vLONG   offset = stack->pop<vLONG>(ctx);
			vCOMMON o = stack->pop<vCOMMON>(ctx);
			vCOMMON* pfld = 0;
			JObject tgt(ctx, o);
			pfld = &tgt[(size_t)(offset)];
			if (opcode != invokestatic) {
				stack->pop<vCOMMON>(ctx);
			}
			stack->push<vINT>(ctx, pfld->i);
		});

		addNative("jdk/internal/misc/Unsafe/putIntVolatile", "(Ljava/lang/Object;JI)V", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vINT target = stack->pop<vINT>(ctx);
			vLONG   offset = stack->pop<vLONG>(ctx);
			vCOMMON o = stack->pop<vCOMMON>(ctx);
			vCOMMON* pfld = 0;
			JObject tgt(ctx, o);
			pfld = &tgt[(size_t)(offset)];
			if (opcode != invokestatic) {
				stack->pop<vCOMMON>(ctx);
			}
			pfld->i = target;
			//stack->push<vOBJECTREF>(ctx, pfld->objref);
		});

		addNative("java/lang/Class/getDeclaredFields0", "(Z)[Ljava/lang/reflect/Field;", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vBYTE flag = stack->pop<vBYTE>(ctx);
			if (opcode != invokestatic) {
				JObject self(ctx, stack->pop<vCOMMON>(ctx));
				JString tgtClass(ctx, self["name"]);
				auto cls = lazyLoad(ctx, tgtClass.str());
				if (!cls) throw std::runtime_error("class not found");
				//printf("Get declared fields for %s, %s\n", tgtClass.str().c_str(), cls(ctx)->getName(ctx));
				V<vNATIVEARRAY> narr = VMAKEGC(vNATIVEARRAY, ctx, ctx, 1, (vUINT)cls(ctx)->_fieldCount);
				for (size_t i = 0; i < cls(ctx)->_fieldCount; i++) {
					vOBJECTREF fldDescRef = createObject(ctx, "java/lang/reflect/Field", true, 20);
					vFIELD& field = cls(ctx)->_fields(ctx, i);
					JObject fld(ctx, fldDescRef);
					fld["clazz"].set(field.cls(ctx)->getJavaClass(ctx, false));
					fld["name"].set(createString(ctx, field.getName(ctx), true, 0, 7));
					//printf("Created field: %s/%s\n", cls(ctx)->getName(ctx), JString(ctx, fld["name"]).str().c_str());
					narr(ctx)->set<vOBJECTREF>(ctx, i, fldDescRef);
				}
				stack->push<vOBJECTREF>(ctx, Ref(narr));
			}
		});

		addNative("jdk/internal/util/SystemProps$Raw/platformProperties", "()[Ljava/lang/String;", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			JArray<vCOMMON> arr(ctx, ctx->_propsIndices.size(), 1);
			for (int i = 0; i < (int)ctx->_propsIndices.size(); i++) {
				std::string k = ctx->_propsIndices[i];
				arr[i].objref = createString(ctx, ctx->_propsMap[k].c_str(), true, 0, 3);
			}
			stack->push<vOBJECTREF>(ctx, arr.ref());
		});

		addNative("jdk/internal/util/SystemProps$Raw/propDefault", "(I)Ljava/lang/String;", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vINT idx = stack->pop<vINT>(ctx);
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			//printf("Prop default for %d, indices size: %llu\n", idx, ctx->_propsIndices.size());
			std::string k = ctx->_propsIndices[idx];
			//printf("Prop key: %s\n", k.c_str());
			stack->push<vOBJECTREF>(ctx, createString(ctx, ctx->_propsMap[k].c_str(), true, 0, 4));
		});

		addNative("jdk/internal/util/SystemProps$Raw/vmProperties", "()[Ljava/lang/String;", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);

			JArray<vCOMMON> arr(ctx, ctx->_propsPairs.size(), 1);
			for (size_t i = 0; i < ctx->_propsPairs.size(); i++) {
				//std::cout << strarr[i] << std::endl;
				arr[i].objref = createString(ctx, ctx->_propsPairs[i], true, 0, 5);
			}

			stack->push<vOBJECTREF>(ctx, arr.ref());
		});
		
		addNative("jdk/internal/misc/VM/initialize", "()V", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		});

		addNative("java/lang/Runtime/maxMemory", "()J", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			stack->push<vULONG>(ctx, (vULONG)ctx->getAllocator()->getSize());
		});

		addNative("jdk/internal/misc/CDS/isDumpingClassList0", "()Z", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			stack->push<vBYTE>(ctx, 0);
		});

		addNative("jdk/internal/misc/CDS/isDumpingArchive0", "()Z", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			stack->push<vBYTE>(ctx, 0);
		});

		addNative("jdk/internal/misc/CDS/isSharingEnabled0", "()Z", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			stack->push<vBYTE>(ctx, 0);
		});

		addNative("jdk/internal/misc/CDS/initializeFromArchive", "(Ljava/lang/Class;)V", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			stack->pop<vCOMMON>(ctx);
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		});

		addNative("java/lang/Runtime/availableProcessors", "()I", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			stack->push<vINT>(ctx, 1);
		});

		addNative("jdk/internal/misc/Unsafe/storeFence", "()V", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		});

		addNative("java/io/FileInputStream/initIDs", "()V", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		});

		addNative("java/io/FileOutputStream/initIDs", "()V", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		});

		addNative("java/io/FileDescriptor/initIDs", "()V", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		});

		//java/lang/System/setIn0:(Ljava/io/InputStream;)V
		addNative("java/lang/System/setIn0", "(Ljava/io/InputStream;)V", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			stack->pop<vCOMMON>(ctx);
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		});
		
		addNative("java/lang/System/setOut0", "(Ljava/io/PrintStream;)V", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vCOMMON stream = stack->pop<vCOMMON>(ctx);
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			JClass klass(ctx, cpu->lazyLoad(ctx, "java/lang/System"));
			klass["out"].value.set<vCOMMON>(stream);
		});//java/lang/System/setOut0:(Ljava/io/PrintStream;)V

		addNative("java/lang/System/setErr0", "(Ljava/io/PrintStream;)V", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			stack->pop<vCOMMON>(ctx);
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
		});

		//jdk/internal/misc/CDS/getRandomSeedForDumping:()J
		addNative("jdk/internal/misc/CDS/getRandomSeedForDumping", "()J", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			stack->push<vLONG>(ctx, 0x1122334444332211LL);
		});

		// java/lang/Object/hashCode:()I
		addNative("java/lang/Object/hashCode", "()I", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			int hash = 0x55AA55AA;
			if (opcode != invokestatic) {
				vCOMMON comm = stack->pop<vCOMMON>(ctx);
				hash ^= comm.i;
			}
			stack->push<vINT>(ctx, hash);
		});
		
		addNative("java/lang/Object/notifyAll", "()V", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) {
				stack->pop<vCOMMON>(ctx);
			}
			//stack->push<vINT>(ctx, hash);
		}); //java/lang/Object/notifyAll:()V


		addNative("java/lang/Object/getClass", "()Ljava/lang/Class;", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) {
				vCOMMON comm = stack->pop<vCOMMON>(ctx);
				JObject obj(ctx, comm);
				auto saneName = std::string(obj.getClass()(ctx)->getName(ctx));
				auto& ref = getJavaClass(ctx, saneName.c_str(), 0);
				stack->push<vOBJECTREF>(ctx, ref);
				return;
			}
			throw std::runtime_error("invalid object");
		});

		//java/lang/Class/isArray:()Z
		addNative("java/lang/Class/isArray", "()Z", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) {
				vCOMMON comm = stack->pop<vCOMMON>(ctx);
				JObject klass(ctx, comm);
				JString name(ctx, klass["name"]);
				stack->push<vBYTE>(ctx, name.str()[0] == '[');
				return;
			}
			throw std::runtime_error("invalid object");
		});

		addNative("java/lang/Class/isPrimitive", "()Z", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) {
				vCOMMON comm = stack->pop<vCOMMON>(ctx);
				JObject klass(ctx, comm);
				JString name(ctx, klass["name"]);
				std::string n = name.str();
				stack->push<vBYTE>(ctx, n == "int" || n == "byte" || n == "boolean" || n == "long" || n == "short");
				return;
			}
			throw std::runtime_error("invalid object");
		});

		//jdk/internal/reflect/Reflection/getCallerClass:()Ljava/lang/Class
		addNative("jdk/internal/reflect/Reflection/getCallerClass", "()Ljava/lang/Class;", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) {
				stack->pop<vCOMMON>(ctx);
			}
			stack->push<vOBJECTREF>(ctx, getJavaClass(ctx, "java/lang/Object", 0));
		});

		//java/io/FileDescriptor/getHandle:(I)J
		addNative("java/io/FileDescriptor/getHandle", "(I)J", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			int h = stack->pop<vINT>(ctx);
			if (opcode != invokestatic) {
				stack->pop<vCOMMON>(ctx);
			}
			stack->push<vLONG>(ctx, (vLONG)h);
		});

		addNative("java/io/FileDescriptor/getAppend", "(I)Z", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			int h = stack->pop<vINT>(ctx);
			if (opcode != invokestatic) {
				stack->pop<vCOMMON>(ctx);
			}
			stack->push<vBYTE>(ctx, 0);
		});

		//jdk/internal/misc/Signal/findSignal0:(Ljava/lang/String;)I
		addNative("jdk/internal/misc/Signal/findSignal0", "(Ljava/lang/String;)I", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vCOMMON str = stack->pop<vCOMMON>(ctx);
			if (opcode != invokestatic) {
				stack->pop<vCOMMON>(ctx);
			}
			stack->push<vINT>(ctx, 1);
		});

		//jdk/internal/misc/Signal/handle0:(IJ)J
		addNative("jdk/internal/misc/Signal/handle0", "(IJ)J", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vINT i = stack->pop<vINT>(ctx);
			vLONG l = stack->pop<vLONG>(ctx);
			if (opcode != invokestatic) {
				stack->pop<vCOMMON>(ctx);
			}
			stack->push<vLONG>(ctx, l);
		});

		//sun/io/Win32ErrorMode/setErrorMode:(J)J
		addNative("sun/io/Win32ErrorMode/setErrorMode", "(J)J", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vLONG l = stack->pop<vLONG>(ctx);
			if (opcode != invokestatic) {
				stack->pop<vCOMMON>(ctx);
			}
			stack->push<vLONG>(ctx, l);
		});

		addNative("[I/clone", "()Ljava/lang/Object;", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) {
				vCOMMON self = stack->pop<vCOMMON>(ctx);
				JArray<vINT> arr(ctx, self);
				auto clone = arr.clone();
				stack->push<vOBJECTREF>(ctx, clone.ref());
			}
		});

		addNative("java/lang/StrictMath/log", "(D)D", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vDOUBLE n = stack->pop<vDOUBLE>(ctx);
			if (opcode != invokestatic) {
				stack->pop<vCOMMON>(ctx);
			}
			n = std::log(n);
			stack->push<vDOUBLE>(ctx, n);
		});
	}

}