#ifdef POLLEN_STUFF
  {
    double alpha, beta;
    char *alpha_str = getenv("POLLEN_ALPHA"), *beta_str = getenv("POLLEN_BETA");
    
    if (alpha_str && beta_str) {
      alpha = atof(alpha_str);  
      beta = atof(beta_str);
     
      if (alpha < -M_PI || alpha >= M_PI) {
        fprintf(stderr, "%s: pollen - alpha %f out of range\n", progname, alpha);
        exit(1);
      }
      
      if (beta < -M_PI || beta >= M_PI) {
        fprintf(stderr, "%s: pollen - beta %f out of range\n", progname, beta);
        exit(1);
      }
      
      if (verbose > 7)
        fprintf(stderr, "%s: pollen - alpha %f, beta %f\n", progname, alpha, beta);
      
      dwt_pollen_filter(alpha, beta);
    }
  }
#endif

