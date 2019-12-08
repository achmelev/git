#include "builtin.h"
#include "refs.h"
#include "cache.h"




int cmd_apply_diff(int argc, const char **argv, const char *prefix)
{
    
    const char *branch;
    struct object_id head_oid, arg_oid;

    branch = resolve_refdup("HEAD", 0, &head_oid, NULL);

    printf("Head branch = %s, Head commit = %s\n", branch, oid_to_hex(&head_oid));

    
    if (argc > 1) 
    {
        if (!get_oid(argv[1], &arg_oid)) 
        {
            printf("oid of %s is %s\n",argv[1],oid_to_hex(&arg_oid));
        } 
        else {
            printf("Couldn't get oid for %s\n", argv[1]);
        }
    }

	return 0;   
}