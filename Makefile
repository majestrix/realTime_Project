parentOBJS = parent.o ipc_functions.o
doctorOBJS = doctor.o ipc_functions.o
OBJS = $(parentOBJS) $(doctorOBJS)
EXE = doctor parent
FLG = -g -Wall   

all : parent doctor

parent : $(parentOBJS)
	cc $(FLG) -o $@ $(parentOBJS)

doctor : $(doctorOBJS)
	cc $(FLG) -o $@ $(doctorOBJS)

parent.o : local.h ipc_functions.h
doctor.o : local.h ipc_functions.h
ipc_functions.o : local.h 

.PHONY: clean
clean :
	rm $(EXE) $(OBJS)
