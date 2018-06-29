#include "sha1.h"
#include "sha256.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <getopt.h>

enum FuncType {
    FT_SHA1,
    FT_SHA256,
    FT_NONE
};

enum AnalysisType {
    AT_PREIMAGE,
    AT_COLLISION,
    AT_NONE
};

/* config options */
int cfg_use_xor_clauses;
int cfg_use_normal_adders;
int cfg_use_rand;
FuncType cfg_function;
AnalysisType cfg_analysis;


void preimage(int rounds)
{
    if ( cfg_function == FT_SHA1 )
    {
        SHA1 f(rounds, true, cfg_use_xor_clauses, cfg_use_normal_adders);

        unsigned w[80];
        unsigned hash[5];

        if ( cfg_use_rand )
        {
            /* Generate a random pair of input/target */
            for( int i=0; i<16; i++ )
                w[i] = lrand48();

            sha1_comp(w, hash, rounds);
        }
        else
        {
            /* Read the randomly generated input and its hash */
            for( int i=0; i<16; i++ )
                scanf("%x", &w[i]);

            for( int i=0; i<5; i++ )
                scanf("%x", &hash[i]);

            /* Double checking the message words with hash target */
            unsigned h[5];
            sha1_comp(w, h, rounds);
            assert( hash[0] == h[0] );
            assert( hash[1] == h[1] );
            assert( hash[2] == h[2] );
            assert( hash[3] == h[3] );
            assert( hash[4] == h[4] );
        }

        /* Set hash target bits */
        f.fixOutput(hash);

        /* Printing out the instance */
        f.cnf.dimacs();
    }
    else if ( cfg_function == FT_SHA256 )
    {
        SHA256 f(rounds, true, cfg_use_xor_clauses, cfg_use_normal_adders);

        unsigned w[64];
        unsigned hash[8];

        if ( cfg_use_rand )
        {
            /* Generate a random pair of input/target */
            for( int i=0; i<16; i++ )
                w[i] = lrand48();

            sha256_comp(w, hash, rounds);
        }
        else
        {
            /* Read the randomly generated input and its hash */
            for( int i=0; i<16; i++ )
                scanf("%x", &w[i]);

            for( int i=0; i<8; i++ )
                scanf("%x", &hash[i]);

            /* Double checking the message words with hash target */
            unsigned h[8];
            sha256_comp(w, h, rounds);
            assert( hash[0] == h[0] );
            assert( hash[1] == h[1] );
            assert( hash[2] == h[2] );
            assert( hash[3] == h[3] );
            assert( hash[4] == h[4] );
            assert( hash[5] == h[5] );
            assert( hash[6] == h[6] );
            assert( hash[7] == h[7] );
        }


        /* Set hash target bits */
        f.fixOutput(hash);

        /* Printing out the instance */
        f.cnf.dimacs();
    }
    else
    {
        fprintf(stderr, "Invalid function type!\n");
    }
}

void display_usage()
{
    printf("USAGE: ./main {number_of_rounds}\n"
            "  --help or -h                             Prints this message\n"
            "  --xor                                    Use XOR clauses\n"
            "  --normal_adders                          Use normal Teitin encoding for adders (instead of espresso adders)\n"
            "  --random_target                          Generate a random input/target pair (instead of reading from stdin\n"
            "  --rounds or -r {int(16..80)}             Number of rounds in your function\n"
            "  --function or -f {sha1 | sha256}         Type of function under analysis (default: sha1)\n"
            "  --analysis or -a {preimage | collision}  Type of analysis (default: preimage)\n"
          );
}


int main(int argc, char **argv)
{
    unsigned long seed = time( NULL );

    /* Arguments default values */
    cfg_use_xor_clauses = 0;
    cfg_use_normal_adders = 0;
    cfg_use_rand = 0;
    cfg_function = FT_SHA1;
    cfg_analysis = AT_PREIMAGE;
    int rounds = -1;

    struct option long_options[] =
    {
        /* flag options */
        {"xor",           no_argument, &cfg_use_xor_clauses,   1},
        {"normal_adder",  no_argument, &cfg_use_normal_adders, 1},
        {"random_target", no_argument, &cfg_use_rand,          1},
        /* valued options */
        {"rounds",   required_argument, 0, 'r'},
        {"function", required_argument, 0, 'f'},
        {"analysis", required_argument, 0, 'a'},
        {"help",     no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    /* Process command line */
    int c, option_index;
    while( (c = getopt_long(argc, argv, "a:r:f:h", long_options, &option_index)) != -1 )
    {
        switch ( c )
        {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if ( long_options[option_index].flag != 0 )
                    break;
                printf("option %s", long_options[option_index].name);
                if ( optarg )
                    printf (" with arg %s", optarg);
                printf ("\n");
                break;

            case 'r':
                rounds = atoi(optarg);
                break;

            case 'a':
                cfg_analysis = strcmp(optarg, "preimage") == 0 ? AT_PREIMAGE :
                    strcmp(optarg, "collision") == 0 ? AT_COLLISION : 
                    AT_NONE;
                if ( cfg_analysis == AT_NONE )
                {
                    fprintf(stderr, "Invalid or missing analysis type!\nUse -a or --analysis");
                    return 1;
                }
                break;

            case 'f':
                cfg_function = strcmp(optarg, "sha1") == 0 ? FT_SHA1 :
                    strcmp(optarg, "sha256") == 0 ? FT_SHA256 : 
                    FT_NONE;
                if ( cfg_function == FT_NONE )
                {
                    fprintf(stderr, "Invalid or missing function type!\nUse -f or --function");
                    return 1;
                }
                break;

            case 'h':
                display_usage();
                return 1;

            case '?':
/*                if (optopt == 'r')
                      fprintf (stderr, "Please specify the number of rounds with -r.\n");
                  else if (isprint (optopt))
                      fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                  else
                      fprintf (stderr,
                              "Unknown option character `\\x%x'.\n",
                              optopt);*/
                return 1;

            default:
                abort ();
        }
    }

    if ( rounds == -1 )
    {
        fprintf(stderr, "Number of rounds is required! Use -r or --rounds\n");
        return 1;
    }

    srand(seed);
    srand48(rand());

    if ( cfg_analysis == AT_PREIMAGE )
        preimage(rounds);
    else
    {
        printf("Not supported yet!\n");
    }


    return 0;
}