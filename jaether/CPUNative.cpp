#include "CPU.h"
#include "ObjectHelper.h"
#include <fstream>
#include <chrono>
#include <filesystem>
#include <stack>

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
				obj["name"].set<vOBJECTREF>(createString(ctx, stack, L"CurrentThread", 0, 0, false));
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

			auto cls = lazyLoad(ctx, "java/lang/System");
			if (cls) {
				cls(ctx)->invoke(ctx, cls, 0, cpu, stack, opcode, "initPhase1", "()V");
				printf("System initialized\n");
			}
		});

		addNative("java/lang/Class/registerNatives", "()V", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
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
			stack->push<vBYTE>(ctx, 1);
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
			auto& ref = getJavaClass(ctx, stack, str.str().c_str(), &arg);
			stack->push<vOBJECTREF>(ctx, ref);
		});

		addNative("java/lang/System/currentTimeMillis", "()J", [](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			//std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
			//vLONG millis = (vLONG)ms.count();
			stack->push<vLONG>(ctx, ctx->ops() / 50000);
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
			stack->push<vINT>(ctx, 16);
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
			std::string saneName;
			std::string tmp = name.str();
			for (auto c : tmp) {
				if (c == '.') saneName += '/';
				else saneName += c;
			}
			auto klass = lazyLoad(ctx, saneName, 20);

			for (int i = 0; i < klass(ctx)->_fieldCount; i++) {
				vFIELD& fld = klass(ctx)->_fields(ctx, i);
				V<vClass> fldcls(fld.cls);
				std::string currentName(
					(const char*)fldcls(ctx)->toString(ctx, fld.name)(ctx)->s(ctx)
				);
				if (currentName == fieldName.str()) {
					stack->push<vLONG>(ctx, (vLONG)i);
					return;
				}
			}
			printf("%s doesnt have field %s\n", saneName.c_str(), fieldName.str().c_str());
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

			auto& ref = getJavaClass(ctx, stack, saneName.c_str(), &classNameRef);

			stack->push<vOBJECTREF>(ctx, ref);
		});

		//java/security/AccessController/getStackAccessControlContext:()Ljava/security/AccessControlContext;

		addNative("java/security/AccessController/getStackAccessControlContext", "()Ljava/security/AccessControlContext;", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			auto& ref = getObject<vOBJECTREF>(ctx, "java/security/AccessController/getStackAccessControlContext", [this](vContext* ctx) -> vOBJECTREF {
				auto obj = createObject(ctx, "java/security/AccessControlContext", false, 20);
				JObject wrap(ctx, obj);
				wrap.x().set<vLONG>(1011);
				return obj;
			});
			stack->push<vOBJECTREF>(ctx, ref);
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
			JObject tgt(ctx, o);
			vCOMMON& fld = tgt[(size_t)offset];
			printf("Compare and set reference for %s, field: %lld\n", tgt.getClass()(ctx)->getName(ctx), offset);
			if (opcode != invokestatic) {
				stack->pop<vCOMMON>(ctx);
			}
			bool updated = fld.objref.r.a == expected.objref.r.a;
			if (updated) {
				fld.objref.r.a = x.objref.r.a;
			}
			stack->push<vBYTE>(ctx, updated);
		});

		addNative("java/lang/Class/getDeclaredFields0", "(Z)[Ljava/lang/reflect/Field;", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vBYTE flag = stack->pop<vBYTE>(ctx);
			if (opcode != invokestatic) {
				JObject self(ctx, stack->pop<vCOMMON>(ctx));
				auto cls = self.getClass()(ctx);
				V<vNATIVEARRAY> narr = VMAKEGC(vNATIVEARRAY, ctx, ctx, 1, (vUINT)cls->_fieldCount);
				for (size_t i = 0; i < cls->_fieldCount; i++) {
					vOBJECTREF fldDescRef = createObject(ctx, "java/lang/reflect/Field", true, 20);
					vFIELD& field = cls->_fields(ctx, i);
					JObject fld(ctx, fldDescRef);
					fld["clazz"].set(field.cls(ctx)->getJavaClass(this, ctx, stack, false));
					fld["name"].set(createString(ctx, stack, field.getName(ctx)));
					printf("Created field: %s\n", JString(ctx, fld["name"]).str().c_str());
					narr(ctx)->set<vOBJECTREF>(ctx, i, fldDescRef);
				}
				stack->push<vOBJECTREF>(ctx, Ref(narr));
			}
		});

		addNative("jdk/internal/util/SystemProps$Raw/platformProperties", "()[Ljava/lang/String;", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			JArray<vCOMMON> arr(ctx, 40, 1);
			for (int i = 0; i < 40; i++) {
				arr[i].objref = createString(ctx, stack, "Hello");
			}
			stack->push<vOBJECTREF>(ctx, arr.ref());
		});

		addNative("jdk/internal/util/SystemProps$Raw/propDefault", "(I)Ljava/lang/String;", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			vINT idx = stack->pop<vINT>(ctx);
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			stack->push<vOBJECTREF>(ctx, createString(ctx, stack, "Default"));
		});

		addNative("jdk/internal/util/SystemProps$Raw/vmProperties", "()[Ljava/lang/String;", [this](vContext* ctx, vCPU* cpu, vStack* stack, vBYTE opcode) {
			if (opcode != invokestatic) stack->pop<vCOMMON>(ctx);
			std::ifstream props("SystemProperties.txt");
			std::string line;
			std::vector<std::string> strarr;
			while (std::getline(props, line)) {
				auto pos = line.find('=');
				strarr.push_back(line.substr(0, pos));
				strarr.push_back(line.substr(pos + 1));
			}

			JArray<vCOMMON> arr(ctx, strarr.size(), 1);
			for (size_t i = 0; i < strarr.size(); i++) {
				//std::cout << strarr[i] << std::endl;
				arr[i].objref = createString(ctx, stack, strarr[i]);
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
	}

}