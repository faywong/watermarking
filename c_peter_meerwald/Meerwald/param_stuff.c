#ifdef PARAM_STUFF
  {
#define MAXNALPHA 32

    double alpha[MAXNALPHA];
    char *alpha_str = getenv("PARAM_ALPHA");
    double alpha_value;
    int alpha_len = 0;

    int param_len[MAXNALPHA];
    char *param_len_str = getenv("PARAM_LEN");
    int param_len_value;
    int param_len_len = 0;
    int param_len_sum = 0;
    
    char buf[1024] = "";
    char *v;

    
    if (alpha_str && strlen(alpha_str) < sizeof(buf)
	&& strcmp(alpha_str, "") ) {

      strcpy(buf, alpha_str);

      v = strtok(buf, "\",; ");
      do {
     
        alpha_value = atof(v);

        if (alpha_value < -M_PI || alpha_value >= M_PI) {
          fprintf(stderr, "%s: parametric - alpha %f out of range\n",
		  progname, alpha_value);
          exit(1);
        }

        alpha[alpha_len] = alpha_value;
        alpha_len++;
      } while ((v = strtok(NULL, "\",; ")));
      

      if( param_len_str && strlen(param_len_str) < sizeof(buf)
	  && strcmp(param_len_str, "") ) {
	/* There was an parameter length environment variable. */

	strcpy(buf, param_len_str);

	v = strtok(buf, "\",; ");
	do {
     
	  param_len_value = atoi(v);

	  if (param_len_value <= 0) {
	    fprintf(stderr, "%s: parameter length %d out of range\n",
		    progname, param_len_value);
	    exit(1);
	  }

	  param_len[param_len_len] = param_len_value;
	  param_len_len++;
	  param_len_sum += param_len_value;

	} while ((v = strtok(NULL, "\",; ")));

      } else {
	/* No length variable given.
	   For backward compatability we use all parameters for 
	   one filter and therefore for all levels.
	*/

	param_len[0] = alpha_len;
	param_len_len = 1;
	param_len_sum = alpha_len;
      }


      /* If we do not get a parameter length value for every
	 decomposition level then we reuse the last supplied value
	 for the remaining levels.
      */
      if (param_len_len < level+1) {
	int last_param_len = param_len[ param_len_len - 1 ];

	for(; param_len_len < level+1; param_len_len++ ) {
	  param_len[ param_len_len ] = last_param_len;
	  param_len_sum += last_param_len;
	}
      }


      /* If the number of supplied alphas is lower than is required
	 for by param_len then copy the last filter
	 parameters to the remaining levels.
      */
      if( alpha_len < param_len_sum ) {
	int i;
	int last_param_len = param_len[ param_len_len - 1 ];
	int last_start = alpha_len - last_param_len;

	for( i=0; alpha_len < param_len_sum; alpha_len++, i++ ) {
	  alpha[ alpha_len ] = alpha[ last_start + (i % last_param_len) ];
	}
      }

      if (verbose > 1) {
        int i, j;
	int cur_sum = 0;

	fprintf(stderr, "%s: parametric, number of levels: %d\n",
		progname, level);

	for (i = 0; i < level+1; i++) {

	  fprintf(stderr, "    %d filter parameters for level %d: ",
		  param_len[i], i);

	  for (j = 0; j < param_len[i]; j++) {
	    fprintf(stderr, "%f ", alpha[cur_sum + j]);
	  }

	  fprintf(stderr, "\n");

	  cur_sum += param_len[i];
	}
      }


      dwt_param_filter(alpha, param_len);


    } /* if( alpha_str... */
  }
#endif

