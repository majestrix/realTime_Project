parentOBJS = parent.o ipc_functions.o queue.o
doctorOBJS = doctor.o ipc_functions.o queue.o
patientOBJS = patient.o ipc_functions.o queue.o
EXE = doctor parent patient
FLG = -g -Wall   

all : parent doctor patient

parent : $(parentOBJS)
	cc $(FLG) -o $@ $(parentOBJS)

doctor : $(doctorOBJS)
	cc $(FLG) -o $@ $(doctorOBJS)

patient : $(patientOBJS)
	cc $(FLG) -o $@ $(patientOBJS)

parent.o        : local.h ipc_functions.h queue.h
doctor.o        : local.h ipc_functions.h queue.h
patient.o       : local.h ipc_functions.h queue.h
ipc_functions.o : local.h
queue.o         : local.h

.PHONY: clean
clean :
	rm $(EXE) parent.o doctor.o patient.o ipc_functions.o queue.o
