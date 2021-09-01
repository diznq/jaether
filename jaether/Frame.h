#pragma once
#include "Types.h"
#include "Pointer.h"
#include "Stack.h"
#include "Memory.h"
#include "Class.h"

namespace jaether {

	class vFrame {
	public:
		V<vBYTE>	_program;
		V<vStack>	_stack;
		V<vMemory>	_local;
		V<vClass>	_class;
		vULONG		_pc;
		bool		_returns;

		vFrame(
			vContext* ctx,
			vMETHOD* method,
			const V<vClass>& classFile,
			size_t maxStackItems = 64,
			size_t maxLocals = 512
		) {
			_stack = VMAKE(vStack, ctx, ctx, maxStackItems * sizeof(vCOMMON));
			_local = VMAKE(vMemory, ctx, ctx, maxLocals);
			_pc = 0;
			_class = classFile;
			_program = classFile.Ptr(ctx)->getCode(ctx, method);
			if (!_program.IsValid()) {
				fprintf(stderr, "Method %s/%s:%s has no code\n",
					classFile.Ptr(ctx)->getName(ctx),
					classFile.Ptr(ctx)->toString(ctx, method->name).Ptr(ctx)->s.Ptr(ctx),
					classFile.Ptr(ctx)->toString(ctx, method->desc).Ptr(ctx)->s.Ptr(ctx)
				);
			}
			V<vUTF8BODY> desc = classFile.Ptr(ctx)->toString(ctx, method->desc);
			_returns = false;
			if (desc.IsValid()) {
				size_t len = strlen((const char*)desc.Ptr(ctx)->s.Real(ctx));
				_returns = desc.Ptr(ctx)->s[VCtxIdx{ ctx, len - 1 }] != 'V';
			}
		}

		~vFrame() {
		}

		void destroy(vContext* ctx) {
			_stack.Ptr(ctx)->destroy(ctx);
			_stack.Release(ctx);
			_local.Ptr(ctx)->destroy(ctx);
			_local.Release(ctx);
		}

		vBYTE* fetch(vContext* ctx) {
			return _program.Real(ctx) + pc();
		}

		vULONG& pc() {
			return _pc;
		}

		vULONG& incrpc(size_t step) {
			return (_pc) += step;
		}
	};

}