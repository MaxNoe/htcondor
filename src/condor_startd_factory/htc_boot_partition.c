#include "rm_api.h"

int
main(int argc, char **argv) {
	status_t status;
	status_t rmrc;
	rm_BG_t *rmbg;
	int bpNum;
	enum rm_specification getOption;
	rm_BP_t *rmbp;
	rm_bp_id_t bpid;
	rm_BP_state_t state;
	rm_location_t loc;
	rm_partition_list_t *part_list = 0;
	rm_partition_t *part = 0;
	int i;
	int length;
	pm_partition_id_t partID;
	rm_partition_state_t partState;
	char *name;
	rmrc = rm_set_serial("BGP");
	rmrc = rm_get_BG(&rmbg);
	if (rmrc) {
		printf("Error calling rm_get_BG: %d\n", rmrc);
		exit(-1);
	}
	
	printf("argv[1] is %s\n", argv[1]);
	
	status = pm_create_partition(argv[1]);
	printf("status is %d\n", status);
	rm_get_partitions_info(0xff, &part_list);

	rm_get_data(part_list, RM_PartListSize, &length);

	/* printf("There are %d partitions\n", length); */
	for (i = 0; i < length; i++) {
		if (i == 0) {
			rm_get_data(part_list, RM_PartListFirstPart, &part);
		} else {
			rm_get_data(part_list, RM_PartListNextPart, &part);
		}
		rm_get_data(part, RM_PartitionID, &partID);
		rm_get_data(part, RM_PartitionState, &partState);
		name = 0;
		rm_get_data(part, RM_PartitionOptions, &name);
		if (partState != 3) {
			continue;
		}
		printf("%s ", partID);
		switch (name[0]) {
			case 's':
				printf("htc=smp\n");
				break;
			case 'v':
				printf("htc=vn\n");
				break;
			case 'd':
				printf("htc=dual\n");
				break;
		}
	}

	rm_free_BG(rmbg);

	exit(0);
}
