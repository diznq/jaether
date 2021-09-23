# Jaether
## Lightweight JVM implementation in C++

Focus of this project is to be to run simple Java programs in a deterministic way, ensuring that references in memory pools are always same. 

This allows for trust based on multiple machines running same program and computing hash of memory pools every instruction.

## Milestones
- [x] Reflection
- [x] Exceptions
- [x] Somewhat nice way to create strings, objects, arrays in C++ (JClass, JArray, JObject, JString wrappers)
- [x] Switch-case instructions
- [x] Core native methods 
- [x] Hot loading
- [x] initPhase1 loads up correctly
- [ ] Entire JVM instruction set (dup2_x1,2 missing)