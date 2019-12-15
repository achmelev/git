#include "builtin.h"
#include "refs.h"
#include "cache.h"
#include "cache-tree.h"
#include "commit.h"
#include "merge-recursive.h"
#include "sequencer.h"
#include "parse-options.h"
#include "strbuf.h"

static const char commit_editor_comment[] =
N_("Please enter the commit message for your changes. Lines starting\n"
   "with '%c' will be ignored, and an empty message aborts the commit.\n");

static GIT_PATH_FUNC(edit_apply_diff_commit_msg , "EDIT_APPLY_DIFF_COMMIT_MSG")

char* commit_msg = NULL;

static const char * const apply_diff_usage[] = {
	N_("git apply-diff [<options>] [<from>] [<to>]"),
	NULL
};

static struct option apply_diff_options[] = {
	OPT_STRING('m', "message", &commit_msg, N_("message"),
		N_("commit message")),
	OPT_END()
};


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

static void prepare_commit_msg(void) 
{
    struct strbuf commit_msg_buf = STRBUF_INIT;

    strbuf_addch(&commit_msg_buf, '\n');
    strbuf_commented_addf(&commit_msg_buf, commit_editor_comment,comment_line_char);
    write_file_buf(edit_apply_diff_commit_msg(),commit_msg_buf.buf, commit_msg_buf.len);
} 

static void check_conflicts_and_dirty_index(void)
{
	struct strbuf sb = STRBUF_INIT;
    if (repo_read_index_unmerged(the_repository))
    {
        die_resolve_conflict("apply-diff");
    } 

    if (repo_index_has_changes(the_repository, NULL, &sb)) {
	    error(_("your local changes would be overwritten by %s."),"apply-diff");
        advise(_("commit your changes or stash them to proceed."));
        exit(1);
    }    
}


int cmd_apply_diff(int argc, const char **argv, const char *prefix)
{
    
    const char* branch;
    struct commit* head_commit;
    struct object_id head_oid, tree_oid, new_head_oid;
    struct commit* base_commit;
    struct commit* merge_commit;
    struct merge_options merge_opts;
    struct commit_list *parents = NULL;
    const struct object_id *bases[1];
    struct commit *result;
    int clean;
    struct strbuf err = STRBUF_INIT;
    struct strbuf commit_msg_buf = STRBUF_INIT;

    if (argc == 2 && !strcmp(argv[1], "-h"))
		usage_with_options(apply_diff_usage, apply_diff_options);
    
    argc = parse_options(argc, argv, prefix, apply_diff_options,
			apply_diff_usage, 0);

   
    if ((argc == 1) || (argc == 2) ) 
    {
        
        branch = resolve_refdup("HEAD", 0, &head_oid, NULL);
        if (!branch)
            die("apply-diff doesn't work (yet) on a detached HEAD");
    
        head_commit = lookup_commit_or_die(&head_oid, "HEAD");
        
        if (argc == 2) {
            base_commit = get_commit_or_die(argv[0]);
            merge_commit = get_commit_or_die(argv[1]);
        } else {
            base_commit = head_commit;
            merge_commit = get_commit_or_die(argv[0]);
        }    

        check_conflicts_and_dirty_index();

        init_merge_options(&merge_opts, the_repository);
        merge_opts.ancestor = "constructed merge base";
        if (argc == 2) {
            merge_opts.branch1 = argv[0];
            merge_opts.branch2 = argv[1];
        } else {
            merge_opts.branch1 = "HEAD";
            merge_opts.branch2 = argv[0];
        }
        
        bases[0] = &base_commit->object.oid;

        printf("Applying the difference %s .. %s \n",oid_to_hex(&base_commit->object.oid),oid_to_hex(&merge_commit->object.oid));
        clean = merge_recursive_generic(&merge_opts,&head_commit->object.oid,&merge_commit->object.oid,1,bases,&result);
        if (clean < 0) {
            die(_("appy-diff failed"));
            exit(128);
        }
        if (clean == 0) 
        {
            if (write_index_as_tree(&tree_oid, the_repository->index, the_repository->index_file, 0, NULL)) 
            {
		        die(_("git write-tree failed to write a tree"));
            }  
            commit_list_insert(head_commit, &parents);
            if (!commit_msg) {
                prepare_commit_msg();
                if (launch_editor(edit_apply_diff_commit_msg(), &commit_msg_buf, NULL))
                {
			        error(_("Not committing; use 'git commit' to complete.\n"));
	                exit(1);
                }
                strbuf_stripspace(&commit_msg_buf,1);
                if (commit_msg_buf.len == 0) {
                    error(_("Aborting commit due to empty commit message.\n"));
                    exit(1);
                }
                commit_msg = commit_msg_buf.buf;
            }    
            if (commit_tree_extended(commit_msg, strlen(commit_msg), &tree_oid, parents,&new_head_oid, NULL, NULL, NULL)) 
            {
		         die(_("failed to write commit object"));
	        }
            strbuf_addstr(&commit_msg_buf,commit_msg);
            if (update_head_with_reflog(head_commit, &new_head_oid, getenv("GIT_REFLOG_ACTION"), &commit_msg_buf,&err)) {
		        die("%s", err.buf);
	        }
            print_commit_summary(the_repository, NULL, &new_head_oid,SUMMARY_SHOW_AUTHOR_DATE);
        } 
        
    } else {
        usage_with_options(apply_diff_usage, apply_diff_options);
    }

	return 0;   
}