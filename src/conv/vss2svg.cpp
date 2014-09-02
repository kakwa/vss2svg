/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * This file is part of the libvisio project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <librevenge-stream/librevenge-stream.h>
#include <librevenge-generators/librevenge-generators.h>
#include <librevenge/librevenge.h>
#include <libvisio/libvisio.h>
#include <fstream>
#include <string> 
#include <stdlib.h>
#include <argp.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

const char *argp_program_version = "vss2svg 1.0";

const char *argp_program_bug_address = "<carpentier.pf@gmail.com>";

static char doc[] = "vss2svg -- Visio stencil to SVG converter";

     static struct argp_option options[] = {
       {"verbose",  'v', 0,      0,  "Produce verbose output" },
       {"svg",      's', 0,      0,  "Produce svg files (default)" },
       {"yed",      'y', 0,      0,  "Produce yED graphml file (unsupported" },
       {"input",    'i', "FILE", 0,  "Input Visio .vss file"   },
       {"output",   'o', "FILE/DIR", 0, "Output file (yED) or directory (svg)"},
       { 0 }
     };

/* A description of the arguments we accept. */
static char args_doc[] = "ARG1 ARG2";

struct arguments
{
  char *args[2];                /* arg1 & arg2 */
  int  svg, verbose, yed;
  char *output;
  char *input;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = (struct arguments *)state->input;

  switch (key)
    {
    case 's': 
      arguments->svg = 1;
      break;
    case 'y': 
      arguments->yed = 1;
      break;
    case 'v':
      arguments->verbose = 1;
      break;
    case 'o':
      arguments->output = arg;
      break;
    case 'i':
      arguments->input = arg;
      break;


    case ARGP_KEY_ARG:
      if (state->arg_num >= 6)
        /* Too many arguments. */
        argp_usage (state);

      arguments->args[state->arg_num] = arg;

      break;

    case ARGP_KEY_END:
      if (state->arg_num < 0)
        /* Not enough arguments. */
        argp_usage (state);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };

namespace
{

int printUsage()
{
  printf("Usage: vss2svg [OPTION] <Visio Stencils File>\n");
  printf("\n");
  printf("Options:\n");
  printf("--help                Shows this help message\n");
  return -1;
}



} // anonymous namespace

int main(int argc, char *argv[])
{

  struct arguments arguments;
  arguments.svg = 1;
  arguments.yed = 0;
  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  librevenge::RVNGFileStream input(arguments.input);

  if (!libvisio::VisioDocument::isSupported(&input))
  {
    std::cerr << "ERROR: Unsupported file format (unsupported version) or file is encrypted!" << std::endl;
    return 1;
  }

  librevenge::RVNGStringVector output;
  librevenge::RVNGSVGDrawingGenerator generator(output, "svg");
  if (!libvisio::VisioDocument::parseStencils(&input, &generator))
  {
    std::cerr << "ERROR: SVG Generation failed!" << std::endl;
    return 1;
  }
  if (output.empty())
  {
    std::cerr << "ERROR: No SVG document generated!" << std::endl;
    return 1;
  }
  std::string outputdir(arguments.output);
  mkdir(arguments.output, S_IRWXU);
  for (unsigned k = 0; k<output.size(); ++k)
  {
    ofstream myfile;

    std::basic_string<char> newfilename = outputdir + "image-"+ std::to_string(k) + ".svg" ;
    myfile.open (newfilename);
    myfile << output[k].cstr() << std::endl;
    myfile.close();
  }

  return 0;
}
/* vim:set shiftwidth=2 softtabstop=2 expandtab: */
