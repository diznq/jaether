#pragma once
#include "Types.h"
#include "Pointer.h"
#include "Stack.h"
#include "Memory.h"
#include "Class.h"
#include <string>

namespace jaether {

	class vFrame {
	public:
		CodeAttribute	_program;
		V<vStack>	_stack;
		V<vMemory>	_local;
		V<vClass>	_class;
		V<vMETHOD>  _method;
		vULONG		_pc;
		bool		_returns;

		vFrame(
			vContext* ctx,
			vMETHOD* method,
			const V<vClass>& classFile,
			size_t maxStackItems = 64,
			size_t maxLocals = 64
		) {
			_stack = VMAKE(vStack, ctx, ctx, maxStackItems * sizeof(vCOMMON));
			_local = VMAKE(vMemory, ctx, ctx, maxLocals);
			_pc = 0;
			_class = classFile;
			_method = (vMETHOD*)((uintptr_t)method - (uintptr_t)ctx->getAllocator()->getBase());
			_program = classFile(ctx)->getCode(ctx, method);
			if (!_program.code.isValid()) {
				fprintf(stderr, "Method %s/%s:%s has no code\n",
					classFile(ctx)->getName(ctx),
					classFile(ctx)->toString(ctx, method->name)(ctx)->s(ctx),
					classFile(ctx)->toString(ctx, method->desc)(ctx)->s(ctx)
				);
				throw std::runtime_error("method has no code");
			}
			V<vUTF8BODY> desc = classFile(ctx)->toString(ctx, method->desc);
			_returns = false;
			if (desc.isValid()) {
				size_t len = strlen((const char*)desc(ctx)->s.real(ctx));
				_returns = desc(ctx)->s( ctx,  len - 1) != 'V';
			}
		}

		~vFrame() {

		}

		void destroy(vContext* ctx) {
			_stack(ctx)->destroy(ctx);
			_stack.release(ctx);
			_local(ctx)->destroy(ctx);
			_local.release(ctx);
		}

		vBYTE* fetch(vContext* ctx) {
			return _program.code.real(ctx) + pc();
		}

		vULONG& pc() {
			return _pc;
		}

		vULONG& incrpc(size_t step) {
			return (_pc) += step;
		}

		std::string getName(vContext* ctx) const {
			return std::string(_class(ctx)->getName(ctx))
				+ "/" +
				(const char*)_class(ctx)->toString(ctx, _method(ctx)->name)(ctx)->s(ctx)
				+ ":" +
				(const char*)_class(ctx)->toString(ctx, _method(ctx)->desc)(ctx)->s(ctx);
		}
	};

}