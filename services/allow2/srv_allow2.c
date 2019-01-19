/*
 *  Copyright (C) 2018 Allow2 Pty Ltd
 *
 *
 */

#include "common.h"
#include "c-icap.h"
#include "service.h"
#include "header.h"
#include "body.h"
#include "simple_api.h"
#include "debug.h"

int allow2_init_service(ci_service_xdata_t * srv_xdata,
                      struct ci_server_conf *server_conf);
int allow2_check_preview_handler(char *preview_data, int preview_data_len,
                               ci_request_t *);
int allow2_end_of_data_handler(ci_request_t * req);
void *allow2_init_request_data(ci_request_t * req);
void allow2_close_service();
void allow2_release_request_data(void *data);
int allow2_io(char *wbuf, int *wlen, char *rbuf, int *rlen, int iseof,
            ci_request_t * req);


CI_DECLARE_MOD_DATA ci_service_module_t service = {
    "allow2",                         /* mod_name, The module name */
    "allow2 demo service",            /* mod_short_descr,  Module short description */
    ICAP_RESPMOD | ICAP_REQMOD,     /* mod_type, The service type is responce or request modification */
    allow2_init_service,              /* mod_init_service. Service initialization */
    NULL,                           /* post_init_service. Service initialization after c-icap
                    configured. Not used here */
    allow2_close_service,           /* mod_close_service. Called when service shutdowns. */
    allow2_init_request_data,         /* mod_init_request_data */
    allow2_release_request_data,      /* mod_release_request_data */
    allow2_check_preview_handler,     /* mod_check_preview_handler */
    allow2_end_of_data_handler,       /* mod_end_of_data_handler */
    allow2_io,                        /* mod_service_io */
    NULL,
    NULL
};

/*
  The allow2_req_data structure will store the data required to serve an ICAP request.
*/
struct allow2_req_data {
    /*the body data*/
    ci_ring_buf_t *body;
    /*flag for marking the eof*/
    int eof;
};


/* This function will be called when the service loaded  */
int allow2_init_service(ci_service_xdata_t * srv_xdata,
                      struct ci_server_conf *server_conf)
{
    ci_debug_printf(5, "Initialization of allow2 module......\n");

    /*Tell to the icap clients that we can support up to 1024 size of preview data*/
    ci_service_set_preview(srv_xdata, 1024);

    /*Tell to the icap clients that we support 204 responses*/
    ci_service_enable_204(srv_xdata);

    /*Tell to the icap clients to send preview data for all files*/
    ci_service_set_transfer_preview(srv_xdata, "*");

    /*Tell to the icap clients that we want the X-Authenticated-User and X-Authenticated-Groups headers
      which contains the username and the groups in which belongs.  */
    ci_service_set_xopts(srv_xdata,  CI_XAUTHENTICATEDUSER|CI_XAUTHENTICATEDGROUPS);

    return CI_OK;
}

/* This function will be called when the service shutdown */
void allow2_close_service()
{
    ci_debug_printf(5,"Service shutdown!\n");
    /*Nothing to do*/
}

/*This function will be executed when a new request for allow2 service arrives. This function will
  initialize the required structures and data to serve the request.
 */
void *allow2_init_request_data(ci_request_t * req)
{
    struct allow2_req_data *allow2_data;

    /*Allocate memory fot the allow2_data*/
    allow2_data = malloc(sizeof(struct allow2_req_data));
    if (!allow2_data) {
        ci_debug_printf(1, "Memory allocation failed inside allow2_init_request_data!\n");
        return NULL;
    }

    /*If the ICAP request encuspulates a HTTP objects which contains body data
      and not only headers allocate a ci_cached_file_t object to store the body data.
     */
    if (ci_req_hasbody(req))
        allow2_data->body = ci_ring_buf_new(4096);
    else
        allow2_data->body = NULL;

    allow2_data->eof = 0;
    /*Return to the c-icap server the allocated data*/
    return allow2_data;
}

/*This function will be executed after the request served to release allocated data*/
void allow2_release_request_data(void *data)
{
    /*The data points to the allow2_req_data struct we allocated in function allow2_init_service */
    struct allow2_req_data *allow2_data = (struct allow2_req_data *)data;

    /*if we had body data, release the related allocated data*/
    if (allow2_data->body)
        ci_ring_buf_destroy(allow2_data->body);

    free(allow2_data);
}


static int whattodo = 0;
int allow2_check_preview_handler(char *preview_data, int preview_data_len,
                               ci_request_t * req)
{
    ci_off_t content_len;

    /*Get the allow2_req_data we allocated using the  allow2_init_service  function*/
    struct allow2_req_data *allow2_data = ci_service_data(req);

    /*If there are is a Content-Length header in encupsulated Http object read it
     and display a debug message (used here only for debuging purposes)*/
    content_len = ci_http_content_length(req);
    ci_debug_printf(9, "We expect to read :%" PRINTF_OFF_T " body data\n",
                    (CAST_OFF_T) content_len);

    /*If there are not body data in HTTP encapsulated object but only headers
      respond with Allow204 (no modification required) and terminate here the
      ICAP transaction */
    if (!ci_req_hasbody(req))
        return CI_MOD_ALLOW204;

    /*Unlock the request body data so the c-icap server can send data before
      all body data has received */
    ci_req_unlock_data(req);

    /*If there are not preview data tell to the client to continue sending data
      (http object modification required). */
    if (!preview_data_len)
        return CI_MOD_CONTINUE;

    /* In most real world services we should decide here if we must modify/process
    or not the encupsulated HTTP object and return CI_MOD_CONTINUE or
    CI_MOD_ALLOW204 respectively. The decision can be taken examining the http
    object headers or/and the preview_data buffer.
    In this example service we just use the whattodo static variable to decide
    if we want to process or not the HTTP object.
         */
    if (whattodo == 0) {
        whattodo = 1;
        ci_debug_printf(8, "allow2 service will process the request\n");

        /*if we have preview data and we want to proceed with the request processing
          we should store the preview data. There are cases where all the body
          data of the encapsulated HTTP object included in preview data. Someone can use
          the ci_req_hasalldata macro to  identify these cases*/
        if (preview_data_len) {
            ci_ring_buf_write(allow2_data->body, preview_data, preview_data_len);
            allow2_data->eof = ci_req_hasalldata(req);
        }
        return CI_MOD_CONTINUE;
    } else {
        whattodo = 0;
        /*Nothing to do just return an allow204 (No modification) to terminate here
         the ICAP transaction */
        ci_debug_printf(8, "Allow 204...\n");
        return CI_MOD_ALLOW204;
    }
}

/* This function will called if we returned CI_MOD_CONTINUE in  allow2_check_preview_handler
 function, after we read all the data from the ICAP client*/
int allow2_end_of_data_handler(ci_request_t * req)
{
    struct allow2_req_data *allow2_data = ci_service_data(req);
    /*mark the eof*/
    allow2_data->eof = 1;
    /*and return CI_MOD_DONE */
    return CI_MOD_DONE;
}

/* This function will called if we returned CI_MOD_CONTINUE in  allow2_check_preview_handler
   function, when new data arrived from the ICAP client and when the ICAP client is
   ready to get data.
*/
int allow2_io(char *wbuf, int *wlen, char *rbuf, int *rlen, int iseof,
            ci_request_t * req)
{
    int ret;
    struct allow2_req_data *allow2_data = ci_service_data(req);
    ret = CI_OK;

    /*write the data read from icap_client to the allow2_data->body*/
    if (rlen && rbuf) {
        *rlen = ci_ring_buf_write(allow2_data->body, rbuf, *rlen);
        if (*rlen < 0)
            ret = CI_ERROR;
    }

    /*read some data from the allow2_data->body and put them to the write buffer to be send
     to the ICAP client*/
    if (wbuf && wlen) {
        *wlen = ci_ring_buf_read(allow2_data->body, wbuf, *wlen);
    }
    if (*wlen==0 && allow2_data->eof==1)
        *wlen = CI_EOF;

    return ret;
}


/*****************************************************************/
/* c-icap lookup table databases                                 */


//void *lt_load_db(struct lookup_db *db, const char *path)
//{
//  struct ci_lookup_table *lt_db;
//  lt_db = ci_lookup_table_create(path);
//  if(lt_db && !ci_lookup_table_open(lt_db)) {
//    ci_lookup_table_destroy(lt_db);
//    lt_db = NULL;
//  }
//  return (db->db_data = (void *)lt_db);
//}
//
//char *find_last(char *s,char *e,const char accept)
//{
//  char *p;
//  p = e;
//  while(p >= s) {
//      if(accept == *p)
//	  return p;
//      p--;
//  }
//  return NULL;
//}
//
//typedef struct subcats_data cmp_data;
//static int cmp_fn(cmp_data *cmp, const struct subcats_data *cfg)
//{
//    cmp->op = 0;
//    if (cfg->str && cmp->str && strcmp(cmp->str, cfg->str) == 0) {
//        switch(cfg->op) {
//        case SBC_LESS:
//            if (cmp->score < cfg->score)
//                cmp->op = 1; /*matches*/
//            break;
//        case SBC_GREATER:
//            if (cmp->score > cfg->score)
//                cmp->op = 1; /*matches*/
//            break;
//        default:
//            cmp->op = 1;
//            break;
//        }
//        if (cfg->op > 0) {
//            ci_debug_printf(5, "srv_url_check: Matches sub category: %s, requires score: %d%c%d %s matches\n",
//                            cmp->str, cmp->score, cfg->op == SBC_LESS? '<' : '>', cfg->score, cmp->op? "" : "not");
//        } else {
//            ci_debug_printf(5, "srv_url_check: Matches sub category: %s\n", cmp->str);
//        }
//        return cmp->op;
//    }
//    return 0;
//}
//
//static const char *check_sub_categories(const char *key, char **vals, ci_ptr_vector_t *subcats, char *str_cats, size_t str_cats_size)
//{
//    int i, len;
//    char buf[1024], *e;
//    cmp_data cmp;
//    if (!subcats)
//        return key;
//
//    /*if sub-categories defined but no vals returned, do not match */
//    if (!vals)
//        return NULL;
//
//    for (i = 0; vals[i] != NULL; i++) {
//        if ((e = strchr(vals[i], ':')) != NULL) {
//            /*We found a value in the form "value:score".
//              Split score from string, to pass it for check with subcategories.
//            */
//            cmp.score = strtol(e+1, NULL, 10);
//            if (cmp.score <= 0) {
//                cmp.str = vals[i];
//                cmp.score = 0;
//            } else {
//                /* a valid score found */
//                len = e - vals[i];
//                strncpy(buf, vals[i], len);
//                buf[len] = '\0';
//                cmp.str = buf;
//            }
//        } else {
//            cmp.str = vals[i];
//            cmp.score = 0;
//        }
//
//        cmp.op = 0;
//        ci_ptr_vector_iterate(subcats, &cmp, (int (*)(void *, const void *))cmp_fn);
//        if (cmp.op != 0) {
//            strncpy(str_cats, cmp.str, str_cats_size);
//            str_cats[str_cats_size-1] = '\0';
//            return key;
//        }
//    }
//
//    return NULL;
//}
//
//int lt_lookup_db(struct lookup_db *ldb, struct url_check_http_info *http_info, struct match_info *match_info, ci_ptr_vector_t *subcats)
//{
//  char str_subcats[1024];
//  char **vals = NULL;
//  const char *ret = NULL;
//  char *s, *snext, *e, *end, store;
//  int len, full_url = 0;
//  struct ci_lookup_table *lt_db = (struct ci_lookup_table *)ldb->db_data;
//
//  if (!http_info->url) { /*Is not possible*/
//      ci_debug_printf(1, "lt_lookup_db: Null url passed. (Bug?)");
//      return 0;
//  }
//
//  switch(ldb->check) {
//  case CHECK_HOST:
//      ret = ci_lookup_table_search(lt_db, http_info->site, &vals);
//      if (ret) {
//          if (subcats)
//              ret = check_sub_categories(ret, vals, subcats, str_subcats, sizeof(str_subcats));
//          if (vals) {
//              ci_lookup_table_release_result(lt_db, (void **)vals);
//              vals = NULL;
//          }
//      }
//      break;
//  case CHECK_DOMAIN:
//      s = http_info->site;
//      s--;   /* :-) */
//      do {
//	  s++;
//	  ci_debug_printf(5, "srv_url_check: Checking  domain %s ....\n", s);
//	  ret = ci_lookup_table_search(lt_db, s, &vals);
//          if (ret) {
//              if (subcats)
//                  ret = check_sub_categories(ret, vals, subcats, str_subcats, sizeof(str_subcats));
//              if (vals) {
//                  ci_lookup_table_release_result(lt_db, (void **)vals);
//                  vals = NULL;
//              }
//          }
//      } while (!ret && (s = strchr(s, '.')));
//      break;
//  case CHECK_FULL_URL:
//      full_url = 1;
//  case CHECK_URL:
//      /*for www.site.com/to/path/page.html need to test:
//	www.site.com/to/path/page.html
//	www.site.com/to/path/
//	www.site.com/to/
//	www.site.com/
//	site.com/to/path/page.html
//	site.com/to/path/
//	site.com/to/
//	site.com/
//	com/to/path/page.html
//	com/to/path/
//	com/to/
//	com/
//       */
//      s = http_info->url;
//      if (!full_url && http_info->args)
//	  end = http_info->args;
//      else {
//	  len = strlen(http_info->url);
//	  end = s+len;
//      }
//      s--;
//      do {
//	  s++;
//	  e = end; /*Point to the end of string*/
//	  snext = strpbrk(s, "./");
//	  if(!snext || *snext == '/') /*Do not search the top level domains*/
//	      break;
//	  do {
//	      store = *e;
//	      *e = '\0'; /*cut the string exactly here (the http_info->url must not change!) */
//	      ci_debug_printf(9,"srv_url_check: Going to check url: %s\n", s);
//	      ret = ci_lookup_table_search(lt_db, s, &vals);
//              if (ret) {
//                  if (subcats)
//                      ret = check_sub_categories(ret, vals, subcats, str_subcats, sizeof(str_subcats));
//                  if (vals) {
//                      ci_lookup_table_release_result(lt_db, (void **)vals);
//                      vals = NULL;
//                  }
//                  match_info->match_length = strlen(s);
//              }
//
//	      *e = store; /*... and restore string to its previous state :-),
//			    the http_info->url must not change */
//	      if (full_url && e > http_info->args)
//		  e = http_info->args;
//	      else
//		  e = find_last(s, e-1, '/' );
//	  } while(!ret && e);
//      } while (!ret && (s = snext));
//
//      break;
//  case CHECK_SIMPLE_URL:
//      s = http_info->url;
//      ci_debug_printf(5, "srv_url_check: Checking  URL %s ....\n", s);
//      ret = ci_lookup_table_search(lt_db, s, &vals);
//      if (ret) {
//          if (subcats)
//              ret = check_sub_categories(ret, vals, subcats, str_subcats, sizeof(str_subcats));
//          if (vals) {
//              ci_lookup_table_release_result(lt_db, (void **)vals);
//              vals = NULL;
//          }
//      }
//      break;
//
//  case CHECK_SRV_IP:
//      break;
//  case CHECK_SRV_NET:
//      break;
//  default:
//      /*nothing*/
//      break;
//  }
//
//  if (ret) {
//      match_info_append_db(match_info, ldb->name, (subcats!= NULL ? str_subcats : NULL));
//      return 1;
//  }
//
//  return 0;
//}
//
//void lt_release_db(struct lookup_db *ldb)
//{
//  struct ci_lookup_table *lt_db = (struct ci_lookup_table *)ldb->db_data;
//  ci_debug_printf(5, "srv_url_check: Destroy lookup table %s\n", lt_db->path);
//  ci_lookup_table_destroy(lt_db);
//  ldb->db_data = NULL;
//}
//
//
//int cfg_load_lt_db(const char *directive, const char **argv, void *setdata)
//{
//  struct lookup_db *ldb;
//  unsigned int check;
//  const char *db_descr = NULL;
//
//  if (argv == NULL || argv[0] == NULL || argv[1] == NULL || argv[2] == NULL) {
//    ci_debug_printf(1, "srv_url_check: Missing arguments in directive:%s\n", directive);
//    return 0;
//  }
//
//  if(strcmp(argv[1],"host") == 0)
//    check = CHECK_HOST;
//  else if(strcmp(argv[1],"url") == 0)
//    check = CHECK_URL;
//  else if(strcmp(argv[1],"full_url") == 0)
//      check = CHECK_FULL_URL;
//  else if(strcmp(argv[1],"url_simple_check") == 0)
//      check = CHECK_SIMPLE_URL;
//  else if(strcmp(argv[1],"domain") == 0)
//    check = CHECK_DOMAIN;
//  /* Not yet implemented
//  else if(strcmp(argv[1],"server_ip") == 0)
//      check = CHECK_SRV_IP;
//  else if(strcmp(argv[1],"server_net") == 0)
//      check = CHECK_SRV_NET;
//  */
//  else {
//    ci_debug_printf(1, "srv_url_check: Wrong argument %s for directive %s\n",
//		    argv[1], directive);
//    return 0;
//  }
//
//  // if exist 4th argument then it is a database description.
//  if (argv[3] != NULL)
//      db_descr = argv[3];
//
//  ldb = new_lookup_db(argv[0],
//		      db_descr,
//		      DB_LOOKUP,
//		      check,
//		      lt_load_db,
//		      lt_lookup_db,
//		      lt_release_db);
//  if(ldb) {
//    if(!ldb->load_db(ldb, argv[2])) {
//      free(ldb);
//      return 0;
//    }
//    return add_lookup_db(ldb);
//  }
//
//  return 0;
//}