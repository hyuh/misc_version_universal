/*
    Copyright (C) 2010  Guhl

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include "gopt.h"
#include <time.h>
#include <string.h>

#define VERSION_A	1
#define VERSION_B	0

void show_help() {
	fprintf( stderr, "misc_version usage:\n" );
	fprintf( stderr, "misc_version [-h|-?|--help] [-v|--version] [-s|--set_version <VERSION>]\n" );
	fprintf( stderr, "\t-h | -? | --help: display this message\n" );
	fprintf( stderr, "\t-v | --version: display program version\n" );
	fprintf( stderr, "\t-c | --cid <CID>: set the CID in misc to the 8-char long CID\n");
	fprintf( stderr, "\t-g | --getcid <CID>: print the CID in misc\n");
	fprintf( stderr, "\t-s | --set_version <VERSION>:  set the version in misc to the 10-char long VERSION\n" );
	exit(1);
}

int main(int argc, const char **argv) {

	int cid = 0, getcid=0, set_version = 0, help = 0;
	const char* s_set_version;
	const char* s_cid;

	if (argc<=1) {
		show_help();
	}

	if (argc>1) {

		void *options= gopt_sort( & argc, argv, gopt_start(
		  gopt_option( 'h', 0, gopt_shorts( 'h', '?' ), gopt_longs( "help", "HELP" )),
		  gopt_option( 'v', 0, gopt_shorts( 'v' ), gopt_longs( "version" )),
		  gopt_option( 'c', GOPT_ARG, gopt_shorts('c'), gopt_longs("cid")),
		  gopt_option( 'g', 0, gopt_shorts('g'), gopt_longs("getcid")),
		  gopt_option( 's', GOPT_ARG, gopt_shorts( 's' ), gopt_longs( "set_version" ))));

		if( gopt( options, 'h' ) ){
			help = 1;
		}

		if( gopt( options, 'v' ) ){
			fprintf( stdout, "misc_version version: %d.%d\n",VERSION_A,VERSION_B);
			exit (0);
		}
		
		if( gopt(options, 'g') ) {
			getcid=1;
		}
		else {

			if(gopt_arg(options, 'c', &s_cid))
			{
			    // if -c or --cid was specified, check s_cid
			    size_t size;
			    size = strlen(s_cid);
			    if(size != 8)
			    {
				fprintf(stderr, "Error: CID must be a 8 character string. Length of specified string: %d\n", (int)size);
				exit(1);
			    }
			    else
			    {
				cid = 1;
				fprintf(stderr, "--cid set. CID will be changed to: %s\n", s_cid);
			    }
			}

			if( gopt_arg(options, 's', &s_set_version)){
				// if -a or --set_version was specified, check s_set_version
				size_t size;
				size = strlen(s_set_version);
				if (size < 7 || size > 14 ){
					fprintf( stderr, "Error: VERSION must be a 7-14 character string. Length of specified string: %d\n",(int)size);
					exit (1);
				} else {
					set_version = 1;
					fprintf( stderr, "--set_version set. VERSION will be changed to: %s\n",s_set_version);
				}
			}

			else {
				help = 1;
			}
		}

		if (help!=0){
			show_help();
		}
		FILE *fdin, *fdout;
		fdin = fopen("/proc/emmc", "r");
		if ( fdin == NULL ) {
			fprintf(stderr, "Not a valid EMMC device\n");
			return(-1);
		}

		char buffer[256];
		char *line;

		line = fgets(buffer, 256, fdin);

		while ( strstr(line, "\"misc\"") == NULL ) {
			line = fgets(buffer, 256, fdin);
		}
		char *part = strtok( line, ":");
		char *INFILE = malloc(256 * sizeof(char) );
		strcpy(INFILE, "/dev/block/");
		strcat(INFILE, part);
		char *OUTFILE = INFILE;
		fclose(fdin);

		char version[15];
		// initialize maximum length version string to all 0x0 in case the older version was longer
		// version is NULL terminated, if old version was longer it will corrupt new version
		memset(version, '\0', sizeof(version));
		memcpy(version, s_set_version, strlen(s_set_version));


		char *backupFile;
		if ((fdin  = fopen("/sbin/recovery", "r")) != NULL ) {
			fclose(fdin);
			backupFile="/misc_backup.bin";
		} else {
			backupFile="/data/local/misc_backup.bin";
		}

		char ch;

		if ( getcid == 0 ) {
			fprintf(stderr, "Patching and backing up misc partition...\n");
		} else { 
			fprintf(stderr, "Extracting CID from misc partition...\n");
		}
		fdin = fopen(INFILE, "rb");
		if (fdin == NULL){
			fprintf(stderr, "Error opening input file.\n");
			return -1;
		}

		fdout = fopen(backupFile, "wb");
		if (fdout == NULL){
			fprintf(stderr, "Error opening backup file.\n");
			return -1;
		}

		//  create a copy of the partition
		long j=0;
		char g_cid[9];
		g_cid[8] = '\0';

		while(!feof(fdin)) {
			ch = fgetc(fdin);
			if(ferror(fdin)) {
				fprintf(stderr, "Error reading input file.\n");
				exit(1);
			}
			if ( getcid==1 ) {
				if (j>=0x0 && j<=0x7) {
					g_cid[j]=ch;
				}
				if (j==0x7) {
					fprintf(stdout, "%s\n", g_cid);
					exit(0);
				}
			} else {
				if(!feof(fdin)) fputc(ch, fdout);
				if(ferror(fdout)) {
					fprintf(stderr,"Error writing backup file.\n");
					exit(1);
				}
			}
			j++;
		}
		if(fclose(fdin)==EOF) {
			fprintf(stderr, "Error closing input file.\n");
			exit(1);
		}

		if(fclose(fdout)==EOF) {
			fprintf(stderr, "Error closing backup file.\n");
			exit(1);
		}

		//  copy back and patch

		fdin = fopen(backupFile, "rb");
		if (fdin == NULL){
			fprintf(stderr, "Error opening backup file.\n");
			return -1;
		}

		fdout = fopen(OUTFILE, "wb");
		if (fdout == NULL){
			fprintf(stderr, "Error opening output file.\n");
			return -1;
		}

		j = 0;

		while(!feof(fdin)) {
			ch = fgetc(fdin);
			if(ferror(fdin)) {
				fprintf(stderr, "Error reading backup file.\n");
				exit(1);
			}
			// CID
			if ((j>=0x0 && j<=0x7)&& (cid!=0)) {
				ch = s_cid[j];
			}
			// VERSION
			if ((j>=0xa0 && j<=0xae)&& (set_version!=0)) {
				ch = version[j-0xa0];
			}
			if(!feof(fdin)) fputc(ch, fdout);
			if(ferror(fdout)) {
				fprintf(stderr, "Error writing output file.\n");
				exit(1);
			}
			j++;
		}
		if(fclose(fdin)==EOF) {
			fprintf(stderr, "Error closing backup file.\n");
			exit(1);
		}

		if(fclose(fdout)==EOF) {
			fprintf(stderr, "Error closing output file.\n");
			exit(1);
		}

		return 0;
	}
}
