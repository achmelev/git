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
    
    struct commit* head_commit;
    struct commit* base_commit;
    struct commit* merge_commit;
    struct merge_options merge_opts;
    struct commit_list *bases = NULL;
    struct commit *result;
    int clean,i;
   
    if (argc == 4) 
    {
        
        if (repo_read_index_unmerged(the_repository))
		    die_resolve_conflict("merge");
        
        head_commit = get_commit_or_die(argv[1]);
        base_commit = get_commit_or_die(argv[2]);
        merge_commit = get_commit_or_die(argv[3]);
        
        printf("Current repo is %s\n",the_repository->gitdir);
        for (i = 0; i < the_repository->index->cache_nr; i++) {
			printf("%s\n", the_repository->index->cache[i]->name);
		}
        

        init_merge_options(&merge_opts, the_repository);
        merge_opts.ancestor = "constructed merge base";
        merge_opts.branch1 = argv[1];
        merge_opts.branch2 = argv[3];

        printf("Merging %s .. %s onto %s\n",oid_to_hex(&base_commit->object.oid),oid_to_hex(&merge_commit->object.oid),oid_to_hex(&head_commit->object.oid));
        commit_list_insert(base_commit, &bases);
        clean = merge_recursive(&merge_opts,head_commit,merge_commit,bases,&result);
        if (clean < 0) {
            exit(128);
        }
        
    } else {
        die("wrong number of arguments!\n");
    }

	return 0;   
}