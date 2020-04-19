# Real-Time Embedded Systems
We would like to create a multi-processing application that simulates a country ravaged by
the coronavirus pandemic. The objective of the simulation is to see under what conditions
the coronavirus patients have a better survival rate. As you might expect, we’ll make the
simulation simple by considering only a limited number of symptoms (parameters). We’ll
consider the following parameters:
* Number of available doctors: Must be a positive number bigger than 0 and less
than 10,000.

* Coronavirus illness severity of patient: We’ll classify the patients according to
the 3 main symptoms:
   1. Fever,
   2. Cough,
   3. Shortness of breath.
Patients can have any or all of the above 3 symptoms and these are attributed to
patients at random. Thus, worst severity is 3 (all 3 symptoms are present), best
severity is 0 (no symptoms).

* How long have they been ill: That can be a random number picked from an
input interval (e.g. [1 day .. 10 days]). The more the patient has been ill, the bigger
is the illness severity.

* Patient medical history: We’ll consider the following factors:
   1. Blood hypertension,
   2. Heart/respiratory system diseases,
   3. Cancer.
   
Again, patients can have any or all of the above 3 diseases that are attributed to
patients at random. Assume if the patient has blood hypertension, add 1 point to
the severity, if he/she has heart/respiratory system diseases, add 2 points and if the
patient has cancer, add 3 points.

* Age of patients: We consider that the older is the patient, the higher is the severity.
* How long has the patient been waiting: Patients might not get immediate
attention from doctors if none of them is available. The more the patient waits, the
more his/her severity increases.


## To-Dos
### Parent
Task|Progress
-|-
Shared-Memory|✔️
Semaphore|✔️
Patient Generation|❌
Doctor Generation|✔️
Monitor Children|❌
Clean up|✔️

### Doctor
Task|Progress
-|-
Shared-Memory|✔️
Semaphore|✔️
Message Queue|✔️
Handling Patient|❌

### Patient
Task|Progress
-|-
Semaphore|✔️
Message Queue|✔️
Attribute Generation|✔️
