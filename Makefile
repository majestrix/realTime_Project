parentOBJS = parent.o ipc_functions.o
doctorOBJS = doctor.o ipc_functions.o
EXE = doctor parent

all : parent doctor

parent : $(parentOBJS)
	cc -o $@ $(parentOBJS)

doctor : $(doctorOBJS)
	cc -o $@ $(doctorOBJS)

parent.o : local.h ipc_functions.h
doctor.o : local.h ipc_functions.h
ipc_functions.o : local.h 

.PHONY: clean
clean :
	rm $(EXE) $(OBJS)
