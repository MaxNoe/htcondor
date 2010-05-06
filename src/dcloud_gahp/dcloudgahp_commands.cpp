#include <stdio.h>
#include <dcloudapi/dcloudapi.h>
#include <string>
#include <stdlib.h>
#include <strings.h>
#include "dcloudgahp_commands.h"
#include "dcloudgahp_common.h"

DcloudGahpCommand::DcloudGahpCommand(const char *cmd, workerfn workerfunc)
{
    command = cmd;
    workerfunction = workerfunc;
}

static int verify_number_args(const int want, const int actual)
{
    if (actual != want) {
        dcloudprintf("Expected %d args, saw %d args\n", want, actual);
        return FALSE;
    }

    return TRUE;
}

static std::string create_instance_output(int reqid,
                                       struct deltacloud_instance *inst)
{
    struct deltacloud_action *act;
    struct deltacloud_address *addr;
    std::string output_string;

    output_string += itoa(reqid);
    output_string += " NULL ";

    output_string += "id=";
    output_string += inst->id;
    output_string += ' ';

    output_string += "state=";
    output_string += inst->state;
    output_string += ' ';

    output_string += "actions=";
    act = inst->actions;
    while (act != NULL) {
        output_string += act->rel;
        act = act->next;
        if (act != NULL)
            output_string += ',';
    }
    output_string += ' ';

    output_string += "public_addresses=";
    addr = inst->public_addresses;
    while (addr != NULL) {
        output_string += addr->address;
        addr = addr->next;
        if (addr != NULL)
            output_string += ',';
    }
    output_string += ' ';

    output_string += "private_addresses=";
    addr = inst->private_addresses;
    while (addr != NULL) {
        output_string += addr->address;
        addr = addr->next;
        if (addr != NULL)
            output_string += ',';
    }

    output_string += '\n';

    return output_string;
}

/*
 * DCLOUD_VM_SUBMIT <reqid> <url> <user> <password> <image_id> <name> <realm_id> <flavor_id>
 *  where all arguments are required.  <reqid>, <url>, <user>, <password>, and
 *  <image_id> all have to be non-NULL; <name>, <realm_id>, and <flavor_id>
 *  should either be the string "NULL" to let deltacloud pick, or a particular
 *  name, realm_id, or flavor_id to specify.
 */
bool dcloud_start_worker(int argc, char **argv, std::string &output_string)
{
    char *url, *user, *password, *image_id, *name, *realm_id, *flavor_id;
    struct deltacloud_api api;
    struct deltacloud_instance inst;
    int reqid;
    bool ret = FALSE;

    dcloudprintf("called\n");

    if (!verify_number_args(9, argc)) {
        output_string = create_failure(0, "Wrong_Argument_Number");
        return FALSE;
    }

    reqid = atoi(argv[1]);
    url = argv[2];
    user = argv[3];
    password = argv[4];
    image_id = argv[5];
    name = argv[6];
    realm_id = argv[7];
    flavor_id = argv[8];

    if (STRCASEEQ(url, NULLSTRING)) {
        dcloudprintf("URL cannot be NULL\n");
        output_string = create_failure(reqid, "Invalid_URL");
        return FALSE;
    }
    if (STRCASEEQ(user, NULLSTRING)) {
        dcloudprintf("User cannot be NULL\n");
        output_string = create_failure(reqid, "Invalid_User");
        return FALSE;
    }
    if (STRCASEEQ(password, NULLSTRING)) {
        dcloudprintf("Password cannot be NULL\n");
        output_string = create_failure(reqid, "Invalid_Password");
        return FALSE;
    }
    if (STRCASEEQ(image_id, NULLSTRING)) {
        dcloudprintf("Image ID cannot be NULL\n");
        output_string = create_failure(reqid, "Invalid_Image_ID");
        return FALSE;
    }

    if (STRCASEEQ(name, NULLSTRING))
        name = NULL;
    if (STRCASEEQ(realm_id, NULLSTRING))
        realm_id = NULL;
    if (STRCASEEQ(flavor_id, NULLSTRING))
        flavor_id = NULL;

    dcloudprintf("Arguments: reqid %d, url %s, user %s, password %s, image_id %s, name %s, realm_id %s, flavor_id %s\n", reqid, url, user, password, image_id, name, realm_id, flavor_id);

    if (deltacloud_initialize(&api, url, user, password) < 0) {
        dcloudprintf("Could not initialize deltacloud\n");
        output_string = create_failure(reqid, "Deltacloud_Init_Failure");
        return FALSE;
    }

    if (deltacloud_create_instance(&api, image_id, name, realm_id, flavor_id,
                                   &inst) < 0) {
        dcloudprintf("Could not create_instance\n");
        output_string = create_failure(reqid, "Create_Instance_Failure");
        goto cleanup_library;
    }

    output_string = create_instance_output(reqid, &inst);

    deltacloud_free_instance(&inst);

    ret = TRUE;

 cleanup_library:
    deltacloud_free(&api);

    return ret;
}

/*
 * DCLOUD_VM_ACTION <reqid> <url> <user> <password> <instance_id> <action>
 *  where reqid, url, user, password, instance_id, and action have to be non-NULL
 */
bool dcloud_action_worker(int argc, char **argv, std::string &output_string)
{
    char *url, *user, *password, *instance_id, *action;
    struct deltacloud_api api;
    struct deltacloud_instance instance;
    int action_ret;
    int reqid;
    bool ret = FALSE;

    dcloudprintf("called\n");

    if (!verify_number_args(7, argc)) {
        output_string = create_failure(0, "Wrong_Argument_Number");
        return FALSE;
    }

    reqid = atoi(argv[1]);
    url = argv[2];
    user = argv[3];
    password = argv[4];
    instance_id = argv[5];
    action = argv[6];
    if (STRCASEEQ(url, NULLSTRING)) {
        dcloudprintf("URL cannot be NULL\n");
        output_string = create_failure(reqid, "Invalid_URL");
        return FALSE;
    }
    if (STRCASEEQ(user, NULLSTRING)) {
        dcloudprintf("User cannot be NULL\n");
        output_string = create_failure(reqid, "Invalid_User");
        return FALSE;
    }
    if (STRCASEEQ(password, NULLSTRING)) {
        dcloudprintf("Password cannot be NULL\n");
        output_string = create_failure(reqid, "Invalid_Password");
        return FALSE;
    }
    if (STRCASEEQ(instance_id, NULLSTRING)) {
        dcloudprintf("Instance ID cannot be NULL\n");
        output_string = create_failure(reqid, "Invalid_Instance_ID");
        return FALSE;
    }
    if (STRCASEEQ(action, NULLSTRING)) {
        dcloudprintf("Action cannot be NULL\n");
        output_string = create_failure(reqid, "Invalid action");
        return FALSE;
    }

    if (deltacloud_initialize(&api, url, user, password) < 0) {
        dcloudprintf("Could not initialize deltacloud\n");
        output_string = create_failure(reqid, "Deltacloud_Init_Failure");
        return FALSE;
    }

    if (deltacloud_get_instance_by_id(&api, instance_id, &instance) < 0) {
        dcloudprintf("Failed to find instance id %s\n", instance_id);
        output_string = create_failure(reqid, "Instance_Lookup_Failure");
        goto cleanup_library;
    }

    if (STRCASEEQ(action, "STOP"))
        action_ret = deltacloud_instance_stop(&api, &instance);
    else if (STRCASEEQ(action, "REBOOT"))
        action_ret = deltacloud_instance_reboot(&api, &instance);
    else if (STRCASEEQ(action, "START"))
        action_ret = deltacloud_instance_start(&api, &instance);
    else if (STRCASEEQ(action, "DESTROY"))
        action_ret = deltacloud_instance_destroy(&api, &instance);
    else {
        dcloudprintf("Invalid action %s\n", action);
        output_string = create_failure(reqid, "Invalid_Action");
        goto cleanup_instance;
    }

    if (action_ret < 0) {
        dcloudprintf("Failed to perform action on instance %s\n", instance_id);
        output_string = create_failure(reqid, "Action_Failure");
        goto cleanup_instance;
    }

    output_string += itoa(reqid);
    output_string += " NULL\n";

    ret = TRUE;

cleanup_instance:
    deltacloud_free_instance(&instance);

cleanup_library:
    deltacloud_free(&api);

    return ret;
}

/*
 * DCLOUD_VM_INFO <reqid> <url> <user> <password> <instance_id>
 *  where reqid, url, user, password, and instance_id have to be non-NULL
 */
bool dcloud_info_worker(int argc, char **argv, std::string &output_string)
{
    char *url, *user, *password, *instance_id;
    struct deltacloud_api api;
    struct deltacloud_instance inst;
    int reqid;
    bool ret = FALSE;

    dcloudprintf("called\n");

    if (!verify_number_args(6, argc)) {
        output_string = create_failure(0, "Wrong_Argument_Number");
        return FALSE;
    }

    reqid = atoi(argv[1]);
    url = argv[2];
    user = argv[3];
    password = argv[4];
    instance_id = argv[5];
    if (STRCASEEQ(url, NULLSTRING)) {
        dcloudprintf("URL cannot be NULL\n");
        output_string = create_failure(reqid, "Invalid_URL");
        return FALSE;
    }
    if (STRCASEEQ(user, NULLSTRING)) {
        dcloudprintf("User cannot be NULL\n");
        output_string = create_failure(reqid, "Invalid_User");
        return FALSE;
    }
    if (STRCASEEQ(password, NULLSTRING)) {
        dcloudprintf("Password cannot be NULL\n");
        output_string = create_failure(reqid, "Invalid_Password");
        return FALSE;
    }
    if (STRCASEEQ(instance_id, NULLSTRING)) {
        dcloudprintf("Instance ID cannot be NULL\n");
        output_string = create_failure(reqid, "Invalid_Instance_ID");
        return FALSE;
    }

    if (deltacloud_initialize(&api, url, user, password) < 0) {
        dcloudprintf("Could not initialize deltacloud\n");
        output_string = create_failure(reqid, "Deltacloud_Init_Failure");
        return FALSE;
    }

    if (deltacloud_get_instance_by_id(&api, instance_id, &inst) < 0) {
        dcloudprintf("Failed to find instance id %s\n", instance_id);
        output_string = create_failure(reqid, "Instance_Lookup_Failure");
        goto cleanup_library;
    }

    output_string = create_instance_output(reqid, &inst);

    ret = TRUE;

    deltacloud_free_instance(&inst);

cleanup_library:
    deltacloud_free(&api);

    return ret;
}

/*
 * DCLOUD_VM_STATUS_ALL <reqid> <url> <user> <password>
 *  where reqid, url, user, and password have to be non-NULL.
 */
bool dcloud_statusall_worker(int argc, char **argv, std::string &output_string)
{
    char *url, *user, *password;
    struct deltacloud_api api;
    struct deltacloud_instance *instances;
    struct deltacloud_instance *curr;
    int reqid;
    bool ret = FALSE;

    dcloudprintf("called\n");

    if (!verify_number_args(5, argc)) {
        output_string = create_failure(0, "Wrong_Argument_Number");
        return FALSE;
    }

    reqid = atoi(argv[1]);
    url = argv[2];
    user = argv[3];
    password = argv[4];
    if (STRCASEEQ(url, NULLSTRING)) {
        dcloudprintf("URL cannot be NULL\n");
        output_string = create_failure(reqid, "Invalid_URL");
        return FALSE;
    }
    if (STRCASEEQ(user, NULLSTRING)) {
        dcloudprintf("User cannot be NULL\n");
        output_string = create_failure(reqid, "Invalid_User");
        return FALSE;
    }
    if (STRCASEEQ(password, NULLSTRING)) {
        dcloudprintf("Password cannot be NULL\n");
        output_string = create_failure(reqid, "Invalid_Password");
        return FALSE;
    }

    if (deltacloud_initialize(&api, url, user, password) < 0) {
        dcloudprintf("Could not initialize deltacloud\n");
        output_string = create_failure(reqid, "Deltacloud_Init_Failure");
        return FALSE;
    }

    if (deltacloud_get_instances(&api, &instances) < 0) {
        dcloudprintf("Could not get all instances\n");
        output_string = create_failure(reqid, "Instance_Fetch_Failure");
        goto cleanup_library;
    }

    output_string += itoa(reqid);
    output_string += " NULL";

    curr = instances;
    while (curr != NULL) {
        output_string += " ";
        output_string += curr->id;
        output_string += " ";
        output_string += curr->state;
        curr = curr->next;
    }
    output_string += "\n";

    ret = TRUE;

    deltacloud_free_instance_list(&instances);

cleanup_library:
    deltacloud_free(&api);

    return ret;
}

/*
 * DCLOUD_VM_FIND <reqid> <url> <user> <password> <name>
 *  where reqid, url, user, password, and name have to be non-NULL.
 */
bool dcloud_find_worker(int argc, char **argv, std::string &output_string)
{
    char *url, *user, *password, *name;
    int reqid;
    struct deltacloud_api api;
    struct deltacloud_instance inst;
    int rc;
    bool ret = FALSE;

    dcloudprintf("called\n");

    if (!verify_number_args(6, argc)) {
        output_string = create_failure(0, "Wrong_Argument_Number");
        return FALSE;
    }

    reqid = atoi(argv[1]);
    url = argv[2];
    user = argv[3];
    password = argv[4];
    name = argv[5];
    if (STRCASEEQ(url, NULLSTRING)) {
        dcloudprintf("URL cannot be NULL\n");
        output_string = create_failure(reqid, "Invalid_URL");
        return FALSE;
    }
    if (STRCASEEQ(user, NULLSTRING)) {
        dcloudprintf("User cannot be NULL\n");
        output_string = create_failure(reqid, "Invalid_User");
        return FALSE;
    }
    if (STRCASEEQ(password, NULLSTRING)) {
        dcloudprintf("Password cannot be NULL\n");
        output_string = create_failure(reqid, "Invalid_Password");
        return FALSE;
    }
    if (STRCASEEQ(name, NULLSTRING)) {
        dcloudprintf("Name cannot be NULL\n");
        output_string = create_failure(reqid, "Invalid_Name");
        return FALSE;
    }

    if (deltacloud_initialize(&api, url, user, password) < 0) {
        dcloudprintf("Could not initialize deltacloud\n");
        output_string = create_failure(reqid, "Deltacloud_Init_Failure");
        return FALSE;
    }

    rc = deltacloud_get_instance_by_name(&api, name, &inst);
    if (rc < 0 && rc != DELTACLOUD_FIND_ERROR) {
        dcloudprintf("Could not get all instances\n");
        output_string = create_failure(reqid, "Instance_Fetch_Failure");
        goto cleanup_library;
    }

    output_string += itoa(reqid);
    output_string += " NULL ";
    if (rc == 0) {
        output_string += inst.id;
    } else {
        output_string += "NULL";
    }
    output_string += '\n';

    if (rc == 0) {
        deltacloud_free_instance(&inst);
    }

    ret = TRUE;

cleanup_library:
    deltacloud_free(&api);

    return ret;
}
