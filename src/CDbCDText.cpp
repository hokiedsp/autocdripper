static void 
print_cdtext_track_info(CdIo_t *p_cdio, track_t i_track, const char *psz_msg) {
   cdtext_t *p_cdtext = cdio_get_cdtext(p_cdio, i_track);

  if (NULL != p_cdtext) {
    cdtext_field_t i;
    
    printf("%s\n", psz_msg);
    
    for (i=0; i < MAX_CDTEXT_FIELDS; i++) {
      if (p_cdtext->field[i]) {
        printf("\t%s: %s\n", cdtext_field2str(i), p_cdtext->field[i]);
      }
    }
  }
  cdtext_destroy(p_cdtext);
}

static void 
print_cdtext_info(CdIo_t *p_cdio, track_t i_tracks, track_t i_first_track) {
  track_t i_last_track = i_first_track+i_tracks;
  
  print_cdtext_track_info(p_cdio, 0, "\nCD-TEXT for Disc:");
  for ( ; i_first_track < i_last_track; i_first_track++ ) {
    char msg[50];
    sprintf(msg, "CD-TEXT for Track %2d:", i_first_track);
    print_cdtext_track_info(p_cdio, i_first_track, msg);
  }
}

