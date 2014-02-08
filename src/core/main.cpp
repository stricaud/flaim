//********************************************************************
//        ___           ___       ___                       ___
//       /\  \         /\__\     /\  \          ___        /\__\
//      /::\  \       /:/  /    /::\  \        /\  \      /::|  |
//     /:/\:\  \     /:/  /    /:/\:\  \       \:\  \    /:|:|  |
//    /::\~\:\  \   /:/  /    /::\~\:\  \      /::\__\  /:/|:|__|__
//   /:/\:\ \:\__\ /:/__/    /:/\:\ \:\__\  __/:/\/__/ /:/ |::::\__\
//   \/__\:\ \/__/ \:\  \    \/__\:\/:/  / /\/:/  /    \/__/~~/:/  /
//        \:\__\    \:\  \        \::/  /  \::/__/           /:/  /
//         \/__/     \:\  \       /:/  /    \:\__\          /:/  /
//                    \:\__\     /:/  /      \/__/         /:/  /
//                     \/__/     \/__/                     \/__/
//
//
// Copyright © 2005-2006
// The Board of Trustees of the University of Illinois.
// All rights reserved.
//
// Developed by:
//   LAIM Working Group
//   National Center for Supercomputing Applications
//   University of Illinois
//
// Webpage:
//   http://flaim.ncsa.uiuc.edu/
//
// ASCII Art generated using: http://www.network-science.de/ascii/
//
//********************************************************************
// $Id: main.cpp 1051 2008-02-22 21:25:28Z byoo3 $
//
// The entry point into the flaim core code. Loads modules libs,
// starts threads, processes commandline and file configuration
//
//********************************************************************


#include <ctime>
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>
#include <stdlib.h>
#include <list>
#include <map>
#include <string>
#include <sys/types.h>
#include <dirent.h>

#include "flaim.hpp"
#include "policy_manager.h"
#include "anony_engine.hpp"
#include "record.h"
#include "flaim_core.hpp"
#include "basic_preprocessor.h"
#include "basic_postprocessor.h"
#include "preprocessor.h"
#include "postprocessor.h"
#include "core_globals.h"
#include "listed_record.h"
#include "core_list.h"

// #define DEBUG // comment this line out to remove verbose debugging information

// definitions of global variables
// see core_globals.h
CoreList coreList;
Preprocessor *pre = NULL;
Postprocessor *post = NULL;

bool preRecordsBool = false;
pthread_mutex_t preRecordsAvailMutex;
pthread_cond_t postRecordsAvail;

bool postRecordsBool = false;
pthread_mutex_t postRecordsAvailMutex;
pthread_cond_t preRecordsAvail;

const int CACHE_RECORD_SIZE = 100;
const long WAIT_TIME_SECONDS = 1;

// The name of this program
const char* program_name;

// global variable to indicate verbose mode
// FIXME: add to flaim.hpp also
bool verbose = false;


// FLAIM version string
string flaim_version_string = "FLAIM Core Version 0.7.0, ";
string flaim_revision_string = "Revision ";
string flaim_date_string = "Feb 2008";
string flaim_web_string = "Please Refer to the website for license, updates etc. - http://flaim.ncsa.uiuc.edu";

//Default FLAIM_ROOT
const char* const flaim_root_default = "/usr/local/flaim";
char* flaim_root; //store the actual flaim root


//config read from the config file
//declared global to allow use by add_param
map<string, char*> configFileData;


//callback function for using config_reader
int add_param(char* name, char* value);


//function containing the refactored anony specific code
int process_anonymization(map<string, char*>& configData);

//initialize mutexes and condition variables
void initializeSharedStructures();
void releaseSharedStructures();

// declarations of functions used to start threads
void* startPreprocessor(void* data);
void* startPostprocessor(void* data);
void* startAnonyManager(void* data);


// initializeSharedStructures()
// Initializes shared global mutexes and condition variables
//
// Parameters: None
// Returns: Nothing
// Modifies: Changes the state of globals so they can be used
// Notes: Should be called before, well, doing much of anything
void initializeSharedStructures() {
    int errnum;
    
    errnum = pthread_mutex_init(&postRecordsAvailMutex, NULL);
    if (errnum) {
        cerr << "Main: failed to initialize the postRecordsAvailMutex: " << strerror(errnum ) << endl;
        exit(2);
    }
    
    errnum = pthread_mutex_init(&preRecordsAvailMutex, NULL);
    if (errnum) {
        cerr << "Main: failed to initialize the preRecordsAvailMutex: " << strerror(errnum ) << endl;
        exit(2);
    }
    
    errnum = pthread_cond_init(&postRecordsAvail, NULL);
    if (errnum) {
        cerr << "Main: failed to initialize the postRecordsAvail condition variable: " << strerror(errnum ) << endl;
        exit(2);
    }
    
    errnum = pthread_cond_init(&preRecordsAvail, NULL);
    if (errnum) {
        cerr << "Main: failed to initialize the preRecordsAvail condition variable: " << strerror(errnum ) << endl;
        exit(2);
    }
}



// releaseSharedStructures()
// destroy shared global mutexes and condition variables
void releaseSharedStructures() {
    int errnum;
    
    errnum = pthread_mutex_destroy(&postRecordsAvailMutex);
    if (errnum) {
        cerr << "Main: failed to destroy the postRecordsAvailMutex: " << strerror(errnum ) << endl;
    }
    
    errnum = pthread_mutex_destroy(&preRecordsAvailMutex);
    if (errnum) {
        cerr << "Main: failed to destroy the preRecordsAvailMutex: " << strerror(errnum ) << endl;
    }
    
    
    errnum = pthread_cond_destroy(&postRecordsAvail);
    if (errnum) {
        cerr << "Main: failed to destroy the postRecordsAvail condition variable: " << strerror(errnum ) << endl;
    }
    
    
    errnum = pthread_cond_destroy(&preRecordsAvail);
    if (errnum) {
        cerr << "Main: failed to destroy the preRecordsAvail condition variable: " << strerror(errnum ) << endl;
    }
    
}




// Prints usage information for this program to STREAM (typically// stdout or stderr), and exit the program with EXIT_CODE. Does not
// return. Also print err_msg if specified not NULL
void print_usage(FILE* stream, int exit_code, char* err_msg = NULL) {
    if (err_msg != NULL)
        fprintf(stream, "flaim: %s\n", err_msg);
    
    //NOTE: these should be within 80 columns
    fprintf(stream,
            "Usage: flaim [OPTION] [-m module-name] [-p policy] [-i input.log]\n"
            "       [-o output.log]\n"
            "\n"
            "[Quick Reference] -- Please refer to the Manual for complete help !!\n"
            "\n"
            " -h --help : Display this usage information and exit.\n"
            " -i --input <file-name> : Specifies the source log for anonymization. \n"
            "      If unspecified, stdin will be used. However, not all modules support\n"
            "      reading input from stdin. Those that do not support streaming, will\n"
            "      exit and force you to specify a file name.\n"
            " -l --list : List all installed modules.\n"
            " -m --module <module-name> : Load the specified parsing module. The -l \n"
            "      option shows the valid choices. Either this option must be used to \n"
            "      specify a module installed in the default location, or the -M option\n"
            "      must be used.\n"
            " -M --moduleLib <module-lib-path> : Load the module library from given path.\n"
            "      This option is mutually exclusive with -m and used for explicitly \n"
            "      specifying the module location. It is also necessary to specify schema \n"
            "      location using -s when using this option. \n"
            " -o --output <file-name> : Specifies the destination file for anonymized data.\n"
            "      If unspecified, stdout will be used. However, not all modules support\n"
            "      writing outout to stdout. Those that do not support streaming, will exit\n"
            "      and force you to specify a file name.\n"
            " -p --policy <file-name> : The use of this flag is mandatory as it specifies\n"
            "      the location of the user policy.\n"
            " -s --schemaModule <module-schema-file-path> : Load the module schema file \n"
            "      from the file specified with this option. This option is used if and \n"
            "      only if the -M is used.\n"
            " -v --verbose : Print verbose messages to stderr.\n"
            " -V --version : Print version information to stderr and exit.\n"
            " -x --xtraConfig <file-name> : This is used to specify a file containing extra\n"
            "      information to be passed to the parsing module. It is optional and \n"
            "      ignored by most modules.\n", program_name);
    
    exit(exit_code);
}


//shell env var for flaim install path
const char* const flaim_env = "FLAIM_ROOT";

//global runtime param names
const char* const upolicy =     "user_policy";
const char* const mod_name =    "module_name";
const char* const mod_lib =     "module_lib";
const char* const mod_schema =  "module_schema";
const char* const in_log =      "in_log";
const char* const out_log =     "out_log";
const char* const err_out_log =     "err_out_log";
const char* const xtra_cfg =    "xtra_config";
const char* const version =     "version";

//config file param names
const char * const flaim_schema = "flaimSchemaRng";
const char * const stron_xsl    = "schematronXsl";
const char * const mslStron_xsl = "mslToSchematronXsl";


//config file param
const char* const dir_sepa =     "directorySeparator"; //FIXME: NOT USED currently

//flag to indicate whether the cfg file is specified on the cmd line
bool cfg_cmdline = false;

//flag to indicate --list option
bool mod_list = false;

//flag to indicate whether user policy file is specified on the cmd line
bool  user_policy = false;

//function to list installed modules
void disp_mod_list();

//map debug print func
void dump_config(map<string, char*>* data);


// Read command line options
void read_cmd_line_opts(int argc, char ** argv, map<string, char*>& cmd_opts) {
    int c;
    map<string, char*> * cmd_line_opts;
    cmd_line_opts = &cmd_opts;
    
    // specify valid command line option list
    const struct option long_options[] = {
        { "help", 0, NULL, 'h' },
        { "output", 1, NULL, 'o' },
        { "input", 1, NULL, 'i' },
        { "errOut", 1, NULL, 'e' },
        { "module", 1, NULL, 'm' },
        { "moduleLib", 1, NULL, 'M' },
        { "schemaModule", 1, NULL, 's' },
        { "policy", 1, NULL, 'p' },
        { "list", 0, NULL, 'l' },
        { "xtraConfig", 1, NULL, 'x' },
        { "verbose", 0, NULL, 'v' },
        { "version", 0, NULL, 'V' },
        { NULL, 0, NULL, 0 }
    };
    
    const char* const short_options = "ho:i:e:m:M:s:p:lx:vV";
    
    //obtain the input command-line parameters
    opterr = 0;
    while ((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch(c) {
            case 'p':  //user xml policyfile
                (*cmd_line_opts)[upolicy] = optarg;
                user_policy = true;
                break;
                
            case 'm': //log parser module name
                (*cmd_line_opts)[mod_name] = optarg;
                break;
                
            case 'M': //log parser module lib path
                (*cmd_line_opts)[mod_lib] = optarg;
                break;
                
            case 's': //log parser module schema path
                (*cmd_line_opts)[mod_schema] = optarg;
                break;
                
            case 'i':  //input log
                (*cmd_line_opts)[in_log] = optarg;
                break;
                
            case 'o':  //output log
                (*cmd_line_opts)[out_log] = optarg;
                break;
                
            case 'e':  //error output log
                (*cmd_line_opts)[err_out_log] = optarg;
                break;
                
            case 'h':  //command line help
                print_usage(stderr, 0);
                
            case 'l':  //list installed modules on to stderr
                mod_list = true;
                break;
                
            case 'x':  //xtra config file name
                (*cmd_line_opts)[xtra_cfg] = optarg;
                break;
                
            case 'v':  //run in verbose mode
                verbose = true;
                break;
                
            case 'V':  //print version information
                (*cmd_line_opts)[version] = "1";
                break;
                
            case '?':  //illegal event happened
                if (isprint(optopt))
                    print_usage(stderr, 2, "Parameter specification incomplete");
                else
                    print_usage(stderr, 2, "Unknown parameter specified");
                
            default:
                print_usage(stderr, 2, "Illegal operation while reading cmd-line options");
                // exit(2);
        }
    }
    
}


// Main program entry point
int main(int argc, char ** argv) {
            
    char* cptr;
    
    // get the FLAIM_ROOT, the installation directory
    flaim_root = getenv(flaim_env);
    if (flaim_root == NULL) {
        flaim_root = (char *)flaim_root_default; //harmless cast :-p
    }
    
    if (verbose) fprintf(stderr, "flaim: FLAIM_ROOT = \"%s\"\n", flaim_root);
    
    // FIXME: add stream safe functionality
    
    // Remember the name of the program, to incorporate in messages
    program_name = argv[0];
    
    // config set at the command line
    map<string, char*> configData;
    
    // mode using which we are reading mod lib
    int mod_dir_mode = 0;
    
    // read the input command line option
    read_cmd_line_opts(argc, argv, configData);
    
    // display the modules that are installed
    if (mod_list) disp_mod_list();
    
    // Incase verbose was enabled dump the command line to the stderr as well, useful when sending err logs
    if (verbose) {
        fprintf(stderr, "flaim: command line >> ");
        int acount = argc;
        do {
            fprintf(stderr, "%s ", argv[argc-acount]);
        }
        while (--acount);
        fprintf(stderr, "\n");
    }
    
    // for debug
    if (verbose) dump_config(&configData);
    
    // print out the version information
    if (configData[version] != NULL) {
        
#ifdef BUILD_NUMBER
    char rev_num[20];
    sprintf(rev_num, "%d", BUILD_NUMBER);
    flaim_revision_string.append(rev_num);
    flaim_version_string.append(flaim_revision_string);
    flaim_version_string.append(", ");
#endif                
        flaim_version_string.append(flaim_date_string);        
        cerr << flaim_version_string  << endl;        
        cerr << flaim_web_string << endl;
        
        exit(0);
    }
    
    // specify the global schema files
    string schemaDir(flaim_root);
    schemaDir += "/globalSchemas";
    
    
    if (verbose) cerr << "flaim: schema_dir = " << schemaDir << endl;
    
    string flaim_schema_ptr(schemaDir);
    string stron_xsl_ptr(schemaDir);
    string mslStron_xsl_ptr(schemaDir);
    
    flaim_schema_ptr += "/flaim_schema.rng";
    cptr = (char*)flaim_schema_ptr.c_str();
    configFileData[flaim_schema] = new char[strlen(cptr) + 1];
    strcpy (configFileData[flaim_schema], cptr);
    
    if (verbose)
        fprintf(stderr, "flaim: flaim_schema = \"%s\"\n", configFileData[flaim_schema]);
    
    stron_xsl_ptr += "/schematron-basic.xsl";
    // configFileData[stron_xsl] = (char*)stron_xsl_ptr.c_str();
    
    cptr = (char*)stron_xsl_ptr.c_str();
    configFileData[stron_xsl] = new char[strlen(cptr) + 1];
    strcpy (configFileData[stron_xsl], cptr);
    
    if (verbose)
        fprintf(stderr, "flaim: stron_xsl = \"%s\"\n", configFileData[stron_xsl]);
    
    mslStron_xsl_ptr += "/MSL-to-Schematron.xsl";
    // configFileData[mslStron_xsl] = (char*)mslStron_xsl_ptr.c_str();                
            
    cptr = (char*)mslStron_xsl_ptr.c_str();
    configFileData[mslStron_xsl] = new char[strlen(cptr) + 1];
    strcpy (configFileData[mslStron_xsl], cptr);    
    
    if (verbose)
        fprintf(stderr, "flaim: mslStron_xsl = \"%s\"\n", configFileData[mslStron_xsl]);
    
    // decide where to load module
    if (configData[mod_lib] != NULL) {
        if (configData[mod_schema] == NULL) {
            print_usage(stderr, 2,
                    "parser module schema specification compulsory on the cmd line when specifying module lib path.");
        }
        mod_dir_mode = 0;  //-M & -s options are using
    }
    else if (configData[mod_name] != NULL) {
        // make sure schema is not specified
        if (configData[mod_schema] != NULL) {
            print_usage(stderr, 2, "parser module schema specification on the cmd line is ignored when specifying module name.");
        }
        mod_dir_mode = 1;  //-m option is using
    }
    else {
        print_usage(stderr, 2, "Parser module should be specified on the cmd line.");
    }        
    
    // "-m" option is used
    if (mod_dir_mode) {
        // FIXME: This part may be platform dependent !!!! use dir_sepa
        string modLib(flaim_root);
        string modSchema(flaim_root);
        modLib += "/modules/";
        modLib += configData[mod_name];
        modLib += "/lib";
        modLib += configData[mod_name];
        modLib += "_flaim.so";
        modSchema += "/modules/";
        modSchema += configData[mod_name];
        modSchema += "/";
        modSchema += configData[mod_name];
        modSchema += ".modschema.xml";
        // cerr << "mod lib: " << modLib << endl;
        // cerr << "mod schema: " << modSchema << endl;
                
        cptr = (char*)modLib.c_str();
        configData[mod_lib] = new char[strlen(cptr) + 1];
        strcpy (configData[mod_lib], cptr);
        
        cptr = (char*)modSchema.c_str();
        configData[mod_schema] = new char[strlen(cptr) + 1];
        strcpy (configData[mod_schema], cptr);
        
        // configData[mod_schema] = (char*)modSchema.c_str();
        // configData[mod_schema] = new char(strlen(modSchema.c_str()));
        // strcpy (configData[mod_schema], modSchema.c_str())
        // fprintf(stderr, "configData[mod_schema]: %s\n", configData[mod_schema]);
        // fprintf(stderr, "configData[mod_lib]: %s\n", configData[mod_lib]);
        // configData[mod_lib] = (char*)modLib.data();
        // configData[mod_schema] = (char*)modSchema.data();
    }
    
    if (!user_policy) {
        print_usage(stderr, 2, "user policy file should be specified on the cmd line.");
    }
    
    if (verbose) fprintf(stderr, "flaim: xtra module specific params: \n", configData[xtra_cfg]);    
    
    
    /////////////////////////////////////////////////////////////////////////////
    
    // if we made it this far we're actually going to anonymize something
    // this requires use of the shared data structures, so go ahead and
    // initialize them
    int returnCode = 0;
    
    initializeSharedStructures();
    
    returnCode = process_anonymization(configData);
    
    if (mod_dir_mode) {
        delete[] configData[mod_lib];
        delete[] configData[mod_schema];
    }
    delete[] configFileData[mslStron_xsl];
    delete[] configFileData[stron_xsl];
    delete[] configFileData[flaim_schema];
    
    releaseSharedStructures();
    
    /*
     * free(flaim_schema_ptr);
     * free(stron_xsl_ptr);
     * free(mslStron_xsl_ptr);
     */
    return returnCode;
}


//start the parser module, preprocessor, anonyEngine, postprocessor
int process_anonymization(map<string, char*>& configData) {
    //FIXME !!!!!!!!!
    //unsigned long rcdSeq;
    int rcdSeq;
    
    string inputLog, outputLog, errOutLog;
    string anonyField;
    LogParser *logParser;
    
    inputLog = string("");
    outputLog = string("");
    errOutLog = string("");
    anonyField = string("");
    
    if (configData[in_log] != NULL) {
        inputLog = string(configData[in_log]);
    }
    
    if (configData[out_log] != NULL) {
        outputLog = string(configData[out_log]);
    }
    
    if (configData[err_out_log] != NULL) {
        errOutLog = string(configData[err_out_log]);
    }
    
    if (verbose) fprintf(stderr, "flaim: Starting policy manager...\n");
    
    // call the policyManager to parse and validate the user policy file
    // fprintf(stderr, "0configData[mod_lib]: %s\n", configData[mod_lib]);
    // string modSchema (configData[mod_schema]);
    // fprintf(stderr, "0configData[mod_schema]: %s\n", (char*)modSchema.c_str());
    // fprintf(stderr, "0configData[mod_schema]: %s\n", configData[mod_schema]);
    // UserPolicy* userPolicy = PolicyManager(configFileData, configData[upolicy], configData[mod_schema]);
    UserPolicy* userPolicy = PolicyManager(configFileData, configData[upolicy], configData[mod_schema]);
    
    if (verbose) fprintf(stderr, "flaim: Processing user policy to initiate anonymization...\n");
    
    //get the data out of the userPolicy for anonymization
    vector<Field>::iterator fieldIter;
    fieldIter = userPolicy->fields->begin();
    vector<string> * param_names;
    vector<string> * param_values;
    
    vector<AnonyFields> anonyFields;
    while (fieldIter != userPolicy->fields->end()) {
        AnonyFields *anonyFieldItem = new AnonyFields();
        anonyFieldItem->field_name = (*fieldIter).field_name;
        anonyFieldItem->anony_name = (*fieldIter).anony_name;
        
        anonyField += anonyFieldItem->field_name;
        anonyField += string(" ");
        
        param_names = (*fieldIter).param_names;
        param_values = (*fieldIter).param_values;
        for (unsigned int i = 0; i < param_names->size(); i++) {
            anonyFieldItem->param_names.push_back((*param_names)[i]);
            anonyFieldItem->param_values.push_back((*param_values)[i]);
        }
        
        anonyFields.push_back(*anonyFieldItem);
        delete anonyFieldItem;
        fieldIter++;
    }
    
    if (verbose) fprintf(stderr, "flaim: Creating Anony Engine instance \n");
    
    //initialize the anonymization engine object according to the user policy file
    AnonyEngine* anonyEngine = new AnonyEngine(anonyFields);
    
    //LOAD PARSER MODULE
    if (verbose) fprintf(stderr, "flaim: Loading module dyn-lib from: %s\n", configData[mod_lib]);
    
    //load module library
    void* module = dlopen(configData[mod_lib], RTLD_NOW);
    if (!module) {
        cerr << "flaim: could not load module library: " << dlerror() << endl;
        exit(2);
    }
    else {
        if (verbose) fprintf(stderr, "flaim: module loaded successfully.\n");
    }
    
    //load the symbols
    create_t* create_parser = (create_t*) dlsym(module, "create");
    destroy_t* destroy_parser = (destroy_t *) dlsym(module, "destroy");
    
    if (!create_parser || !destroy_parser) {
        cerr << "flaim: could not load module symbols: " << dlerror() << endl;
        exit(2);
    }
    
    //FIXME: pass xtra config into module here !!!
    //create an instance of the logParser class
    logParser = create_parser(inputLog, outputLog, errOutLog, anonyField);
    logParser->returnCode = 0;
    
    // threading begins here
    // initialize a new preprocessor and postprocessor
    pre = new BasicPreprocessor(logParser, anonyFields.size());
    post = new BasicPostprocessor(logParser);
    
    // gentlemen, start your engines!
    pthread_t prethread, postthread, anonythread;
    int preiret, postiret, anonyiret;    
    
    //create three threads
    preiret = pthread_create(&prethread, NULL, startPreprocessor, NULL);
    if (preiret != 0) {
        cerr << "*** ERROR: failed creating preprocessor thread." << endl;
        exit(2);
    }
    
    anonyiret = pthread_create(&anonythread, NULL, startAnonyManager, (void*)anonyEngine);
    if (anonyiret != 0) {
        cerr << "*** ERROR: failed creating anonymization manager thread." << endl;
        exit(2);
    }
    
    postiret = pthread_create(&postthread, NULL, startPostprocessor, NULL);
    if (postiret != 0) {
        cerr << "*** ERROR: failed creating postprocessor thread." << endl;
        exit(2);
    }
    
    //Wait for all the three threads to exit before leaving the main thread
    if (0 != pthread_join(prethread, NULL))
        cerr << "*** ERROR: pthread_join failed waiting for preprocessor" << endl;
    if (0 != pthread_join(anonythread, NULL))
        cerr << "*** ERROR: pthread_join failed waiting for anonymization manager" << endl;
    if (0 != pthread_join(postthread, NULL))
        cerr << "*** ERROR: pthread_join failed waiting for postprocessor" << endl;
    
    logParser->getRecordSeq(rcdSeq);
    int status = logParser->returnCode;
    cerr << "flaim: A total of " << rcdSeq << " records were processed." << endl;
    
    for (int i = userPolicy->fields->size() - 1; i >= 0 ; i--) {
        delete (userPolicy->fields->at(i)).param_names;
        delete (userPolicy->fields->at(i)).param_values;
    }
    
    delete (vector<Field>*)userPolicy->fields;
    
    //deleting objects for userPolicy and anonyEngine
    delete userPolicy;
    delete anonyEngine;
    
    // delete the pre/postprocessors
    delete pre;
    delete post;
    
    // destroy the instance of the iptable parser module class
    destroy_parser(logParser);
    
    // unload the parser module library
    dlclose(module);
    
    return status;
}


//callback function for read_config
/*int add_param(char* name, char* value)
 * {
 * configFileData[name] = value;
 * if (verbose) fprintf(stderr,"flaim: %s = %s\n",name, configFileData[name]);
 * return 1;
 * }*/


//debug func for printing config map
void dump_config(map<string, char*>* data) {
    fprintf(stderr, "flaim: Config Map Dump >>\n");
    map<string, char*>::const_iterator i = (*data).begin();
    
    while(i != (*data).end()) {
        pair<string, char*> p = *i;
        fprintf(stderr, "flaim: %s -> %s\n", p.first.c_str(), p.second);
        i++;
    }
}


// This function displays the list of modules installed.
// It first tries to use the FLAIM_ROOT and then the default install location
// to search for modules in the ${FLAIM_ROOT}/modules directory.
// This func also verifies that the appropriately named module library and
// schema files are present in each of the sub directories
// All this information and any error msgs are displayed to stderr.
//
void disp_mod_list() {
    char* mod_dir = (char*)calloc( (strlen(flaim_root)+10), 1);  //module dir
    sprintf(mod_dir, "%s/modules", flaim_root);
    
    struct dirent *dp;
    DIR *dfd = opendir(mod_dir);
    
    if (dfd != NULL) {
        while ((dp = readdir(dfd)) != NULL) {
            bool lib, sch;
            lib = sch = false;
            
            char *lib_ptr = (char*) calloc( (strlen(mod_dir)+2*strlen(dp->d_name)+20), 1);  //module lib dir
            char *sch_ptr = (char*) calloc( (strlen(mod_dir)+2*strlen(dp->d_name)+20), 1);  //module schema dir
            sprintf(lib_ptr, "%s/%s/lib%s_flaim.so", mod_dir, dp->d_name, dp->d_name);
            sprintf(sch_ptr, "%s/%s/%s.modschema.xml", mod_dir, dp->d_name, dp->d_name);
            
            if (verbose)
                fprintf(stderr, "flaim: testing in \"%s/%s\" for \"%s\" & \"%s\"\n",
                        mod_dir, dp->d_name, lib_ptr, sch_ptr);
            
            FILE *f;
            if ((f = fopen(lib_ptr, "r")) != NULL) { //check if module lib exists
                fclose(f);
                lib = true;
            }
            else if (verbose) perror("flaim: disp_mod_list");
            
            if ((f = fopen(sch_ptr, "r")) != NULL) { //check if module schema exists
                fclose(f);
                sch = true;
            }
            else if (verbose) perror("flaim: disp_mod_list");
            
            if (lib && sch)
                fprintf(stderr, "flaim: Module \"%s\" detected.\n", dp->d_name);
            
            free(lib_ptr);
            free(sch_ptr);
        }
        closedir(dfd);
    }
    else if (verbose) fprintf(stderr, "flaim: invalid module directory \"%s\".\n", mod_dir);
    
    free(mod_dir);
    
    exit(0); //exit the program after displaying the list of modules
}


// void* startPreprocessor(void* data)
// Function that is called during thread creation to run the preprocessor
//
// Parameters: data, which is ignored
// Returns: NULL, for now
// Modifies: Runs the preprocessor
void* startPreprocessor(void* data) {
    if (verbose) fprintf(stderr, "Main: Entering startPreprocessor\n");
    
    if (pre->preprocessLoop())
        cerr << "*** Preprocessor: preprocessor loop failed" << endl;
    else
        cerr << "*** Preprocessor: preprocessor loop successfully exited" << endl;
    
    pthread_exit(NULL);
    // return NULL;
}

// void* startPostprocessor(void* data)
// Function that is called during thread creation to run the postprocessor
//
// Parameters: data, which is ignored
// Returns: NULL, for now, even though this is not particularly robust
// Modifies: Runs the postprocessor
void* startPostprocessor(void* data) {
    if (verbose) fprintf(stderr, "Main: Entering startPostprocessor\n");
    
    if (post->postprocessLoop())
        cerr << "*** Postprocessor: postprocessor loop failed" << endl;
    else
        cerr << "*** Postprocessor: postprocessor loop successfully exited" << endl;
    
    pthread_exit(NULL);
    // return NULL;
}

// void* startAnonyManager(void* data)
// Function which actually executes the anonymization manager
//
// Parameters: data, a pointer to the anonymization engine object to be used
// Returns: NULL
// Modifies: Anonymizes records in the core list
void* startAnonyManager(void* data) {
    if (verbose) fprintf(stderr, "Main: Entering startAnonyManager\n");
    
    AnonyEngine *engine = (AnonyEngine*)(data);
    
    if (engine->managerLoop())
        cerr << "*** AnonyManager: anonyManager loop failed" << endl;
    else
        cerr << "*** AnonyManager: anonyManager loop successfully exited" << endl;
    
    pthread_exit(NULL);
    // return NULL;
}
