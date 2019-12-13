#include "builtin.h"
#include "refs.h"
#include "cache.h"
#include "commit.h"
#include "merge-recursive.h"


static struct commit *get_commit_or_die(const char *ref_name)
{
	struct object_id oid;
    struct commit *c;
    if (get_oid(ref_name, &oid)) 
    {
        die(_("could not parse object '%s'"), ref_name);
    }
    
    c = lookup_commit_reference(the_repository, &oid);

	if (!c)
		die(_("could not parse %s"), ref_name);
	if (!oideq(&oid, &c->object.oid)) {
		die(_("%s %s is not a commit!"),
			ref_name, oid_to_hex(&oid));
	}
	return c;
}


int cmd_apply_diff(int argc, const char **argv, const char *prefix)
{
    
    const char* branch;
    struct commit* head_commit;
    struct object_id head_oid;
    struct commit* base_commit;
    struct commit* merge_commit;
    struct merge_options merge_opts;
    const struct object_id *bases[1];
    struct commit *result;
    int clean;
   
    if ((argc == 3) || (argc == 2) ) 
    {
        
        branch = resolve_refdup("HEAD", 0, &head_oid, NULL);
        if (!branch)
            die("apply-diff doesn't work (yet) on a detached HEAD");
        
        if (repo_read_index_unmerged(the_repository))
        {
		    die_resolve_conflict("apply-diff");
        }    

        head_commit = lookup_commit_or_die(&head_oid, "HEAD");
        
        if (argc == 3) {
            base_commit = get_commit_or_die(argv[1]);
            merge_commit = get_commit_or_die(argv[2]);
        } else {
            base_commit = head_commit;
            merge_commit = get_commit_or_die(argv[1]);
        }    
        
        init_merge_options(&merge_opts, the_repository);
        merge_opts.ancestor = "constructed merge base";
        merge_opts.branch1 = argv[1];
        merge_opts.branch2 = argv[3];
        bases[0] = &base_commit->object.oid;

        printf("Applying the difference %s .. %s \n",oid_to_hex(&base_commit->object.oid),oid_to_hex(&merge_commit->object.oid));
        clean = merge_recursive_generic(&merge_opts,&head_commit->object.oid,&merge_commit->object.oid,1,bases,&result);
        if (clean < 0) {
            exit(128);
        }
        printf("Result %i\n", clean);
        
    } else {
        die("wrong number of arguments!");
    }

	return 0;   
}