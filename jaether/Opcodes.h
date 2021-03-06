#pragma once

namespace jaether {

	enum vOpcode {
		nop = 0,
		aconst_null = 1,
		iconst_m1 = 2,
		iconst_0 = 3,
		iconst_1 = 4,
		iconst_2 = 5,
		iconst_3 = 6,
		iconst_4 = 7,
		iconst_5 = 8,
		lconst_0 = 9,
		lconst_1 = 10,
		fconst_0 = 11,
		fconst_1 = 12,
		fconst_2 = 13,
		dconst_0 = 14,
		dconst_1 = 15,
		bipush = 16,
		sipush = 17,
		ldc = 18,
		ldc_w = 19,
		ldc2_w = 20,
		iload = 21,
		lload = 22,
		fload = 23,
		dload = 24,
		aload = 25,
		iload_0 = 26,
		iload_1 = 27,
		iload_2 = 28,
		iload_3 = 29,
		lload_0 = 30,
		lload_1 = 31,
		lload_2 = 32,
		lload_3 = 33,
		fload_0 = 34,
		fload_1 = 35,
		fload_2 = 36,
		fload_3 = 37,
		dload_0 = 38,
		dload_1 = 39,
		dload_2 = 40,
		dload_3 = 41,
		aload_0 = 42,
		aload_1 = 43,
		aload_2 = 44,
		aload_3 = 45,
		iaload = 46,
		laload = 47,
		faload = 48,
		daload = 49,
		aaload = 50,
		baload = 51,
		caload = 52,
		saload = 53,
		istore = 54,
		lstore = 55,
		fstore = 56,
		dstore = 57,
		astore = 58,
		istore_0 = 59,
		istore_1 = 60,
		istore_2 = 61,
		istore_3 = 62,
		lstore_0 = 63,
		lstore_1 = 64,
		lstore_2 = 65,
		lstore_3 = 66,
		fstore_0 = 67,
		fstore_1 = 68,
		fstore_2 = 69,
		fstore_3 = 70,
		dstore_0 = 71,
		dstore_1 = 72,
		dstore_2 = 73,
		dstore_3 = 74,
		astore_0 = 75,
		astore_1 = 76,
		astore_2 = 77,
		astore_3 = 78,
		iastore = 79,
		lastore = 80,
		fastore = 81,
		dastore = 82,
		aastore = 83,
		bastore = 84,
		castore = 85,
		sastore = 86,
		pop = 87,
		pop2 = 88,
		dup = 89,
		dup_x1 = 90,
		dup_x2 = 91,
		dup2 = 92,
		dup2_x1 = 93,
		dup2_x2 = 94,
		swap = 95,
		iadd = 96,
		ladd = 97,
		fadd = 98,
		dadd = 99,
		isub = 100,
		lsub = 101,
		fsub = 102,
		dsub = 103,
		imul = 104,
		lmul = 105,
		fmul = 106,
		dmul = 107,
		idiv = 108,
		ldiv_ = 109,
		fdiv = 110,
		ddiv = 111,
		irem = 112,
		lrem = 113,
		frem = 114,
		drem = 115,
		ineg = 116,
		lneg = 117,
		fneg = 118,
		dneg = 119,
		ishl = 120,
		lshl = 121,
		ishr = 122,
		lshr = 123,
		iushr = 124,
		lushr = 125,
		iand = 126,
		land = 127,
		ior = 128,
		lor = 129,
		ixor = 130,
		lxor = 131,
		iinc = 132,
		i2l = 133,
		i2f = 134,
		i2d = 135,
		l2i = 136,
		l2f = 137,
		l2d = 138,
		f2i = 139,
		f2l = 140,
		f2d = 141,
		d2i = 142,
		d2l = 143,
		d2f = 144,
		i2b = 145,
		i2c = 146,
		i2s = 147,
		lcmp = 148,
		fcmpl = 149,
		fcmpg = 150,
		dcmpl = 151,
		dcmpg = 152,
		ifeq = 153,
		ifne = 154,
		iflt = 155,
		ifge = 156,
		ifgt = 157,
		ifle = 158,
		if_icmpeq = 159,
		if_icmpne = 160,
		if_icmplt = 161,
		if_icmpge = 162,
		if_icmpgt = 163,
		if_icmple = 164,
		if_acmpeq = 165,
		if_acmpne = 166,
		goto_ = 167,
		jsr = 168,
		ret = 169,
		tableswitch = 170,
		lookupswitch = 171,
		ireturn = 172,
		lreturn = 173,
		freturn = 174,
		dreturn = 175,
		areturn = 176,
		return_ = 177,
		getstatic = 178,
		putstatic = 179,
		getfield = 180,
		putfield = 181,
		invokevirtual = 182,
		invokespecial = 183,
		invokestatic = 184,
		invokeinterface = 185,
		invokedynamic = 186,
		new_ = 187,
		newarray = 188,
		anewarray = 189,
		arraylength = 190,
		athrow = 191,
		checkcast = 192,
		instanceof = 193,
		monitorenter = 194,
		monitorexit = 195,
		wide = 196,
		multianewarray = 197,
		ifnull = 198,
		ifnonnull = 199,
		goto_w = 200,
		jsr_w = 201
	};

	const char Opcodes[][32] = {
		"nop",
		"aconst_null",
		"iconst_m1",
		"iconst_0",
		"iconst_1",
		"iconst_2",
		"iconst_3",
		"iconst_4",
		"iconst_5",
		"lconst_0",
		"lconst_1",
		"fconst_0",
		"fconst_1",
		"fconst_2",
		"dconst_0",
		"dconst_1",
		"bipush",
		"sipush",
		"ldc",
		"ldc_w",
		"ldc2_w",
		"iload",
		"lload",
		"fload",
		"dload",
		"aload",
		"iload_0",
		"iload_1",
		"iload_2",
		"iload_3",
		"lload_0",
		"lload_1",
		"lload_2",
		"lload_3",
		"fload_0",
		"fload_1",
		"fload_2",
		"fload_3",
		"dload_0",
		"dload_1",
		"dload_2",
		"dload_3",
		"aload_0",
		"aload_1",
		"aload_2",
		"aload_3",
		"iaload",
		"laload",
		"faload",
		"daload",
		"aaload",
		"baload",
		"caload",
		"saload",
		"istore",
		"lstore",
		"fstore",
		"dstore",
		"astore",
		"istore_0",
		"istore_1",
		"istore_2",
		"istore_3",
		"lstore_0",
		"lstore_1",
		"lstore_2",
		"lstore_3",
		"fstore_0",
		"fstore_1",
		"fstore_2",
		"fstore_3",
		"dstore_0",
		"dstore_1",
		"dstore_2",
		"dstore_3",
		"astore_0",
		"astore_1",
		"astore_2",
		"astore_3",
		"iastore",
		"lastore",
		"fastore",
		"dastore",
		"aastore",
		"bastore",
		"castore",
		"sastore",
		"pop",
		"pop2",
		"dup",
		"dup_x1",
		"dup_x2",
		"dup2",
		"dup2_x1",
		"dup2_x2",
		"swap",
		"iadd",
		"ladd",
		"fadd",
		"dadd",
		"isub",
		"lsub",
		"fsub",
		"dsub",
		"imul",
		"lmul",
		"fmul",
		"dmul",
		"idiv",
		"ldiv_",
		"fdiv",
		"ddiv",
		"irem",
		"lrem",
		"frem",
		"drem",
		"ineg",
		"lneg",
		"fneg",
		"dneg",
		"ishl",
		"lshl",
		"ishr",
		"lshr",
		"iushr",
		"lushr",
		"iand",
		"land",
		"ior",
		"lor",
		"ixor",
		"lxor",
		"iinc",
		"i2l",
		"i2f",
		"i2d",
		"l2i",
		"l2f",
		"l2d",
		"f2i",
		"f2l",
		"f2d",
		"d2i",
		"d2l",
		"d2f",
		"i2b",
		"i2c",
		"i2s",
		"lcmp",
		"fcmpl",
		"fcmpg",
		"dcmpl",
		"dcmpg",
		"ifeq",
		"ifne",
		"iflt",
		"ifge",
		"ifgt",
		"ifle",
		"if_icmpeq",
		"if_icmpne",
		"if_icmplt",
		"if_icmpge",
		"if_icmpgt",
		"if_icmple",
		"if_acmpeq",
		"if_acmpne",
		"goto_",
		"jsr",
		"ret",
		"tableswitch",
		"lookupswitch",
		"ireturn",
		"lreturn",
		"freturn",
		"dreturn",
		"areturn",
		"return_",
		"getstatic",
		"putstatic",
		"getfield",
		"putfield",
		"invokevirtual",
		"invokespecial",
		"invokestatic",
		"invokeinterface",
		"invokedynamic",
		"new_",
		"newarray",
		"anewarray",
		"arraylength",
		"athrow",
		"checkcast",
		"instanceof",
		"monitorenter",
		"monitorexit",
		"wide",
		"multianewarray",
		"ifnull",
		"ifnonnull",
		"goto_w",
		"jsr_w"
	};

}