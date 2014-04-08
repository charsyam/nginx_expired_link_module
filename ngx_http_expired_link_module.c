
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <time.h>

typedef struct {
    ngx_str_t                  secret;
} ngx_http_expired_link_conf_t;

static void *ngx_http_expired_link_create_conf(ngx_conf_t *cf);
static ngx_int_t ngx_http_expired_link_add_variables(ngx_conf_t *cf);
static char *
ngx_http_expired_link_merge_conf(ngx_conf_t *cf, void *parent, void *child);


static ngx_command_t  ngx_http_expired_link_commands[] = {
    { ngx_string("expired_link_secret"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_http_expired_link_conf_t, secret),
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_expired_link_module_ctx = {
    ngx_http_expired_link_add_variables,    /* preconfiguration */
    NULL,                                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_expired_link_create_conf,      /* create location configuration */
    ngx_http_expired_link_merge_conf        /* merge location configuration */
};


ngx_module_t  ngx_http_expired_link_module = {
    NGX_MODULE_V1,
    &ngx_http_expired_link_module_ctx,      /* module context */
    ngx_http_expired_link_commands,         /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_str_t  ngx_http_expired_link_name = ngx_string("expired_link");

static char *
ngx_http_expired_link_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
    return NGX_CONF_OK;
}

int get_localtime() {
    time_t rawtime;
    struct tm *info;
    int ret = 0;

    time(&rawtime);
    info = localtime(&rawtime);
    
    ret = (info->tm_year+1900)*10000+(info->tm_mon+1)*100+info->tm_mday;
    return ret; 
}

static ngx_int_t
ngx_http_expired_link_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    u_char      *p, *start, *end, *last;
    size_t       len;

    ngx_http_expired_link_conf_t  *conf;
    int expires;

    conf = ngx_http_get_module_loc_conf(r, ngx_http_expired_link_module);

    if (conf) {
        do {

        }while(0);
    }

    p = &r->unparsed_uri.data[1];
    last = r->unparsed_uri.data + r->unparsed_uri.len;

    while (p < last) {
        if (*p++ == '/') {
            start = p;
            goto path_start;
        }
    }

    goto not_found;

path_start:

    while (p < last) {
        if (*p++ == '/') {
            end = p - 1;
            goto url_start;
        }
    }

    goto not_found;

url_start:

    len = last - p;

    if (end - start != 8 || len == 0) {
        goto not_found;
    }

    expires = ngx_atoi(start,8);
    if (expires < get_localtime()) {
        goto not_found;
    }
    v->data = (u_char *)p;
    v->len = len;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;

    return NGX_OK;

not_found:

    v->not_found = 1;

    return NGX_OK;
}

static void *
ngx_http_expired_link_create_conf(ngx_conf_t *cf)
{
    ngx_http_expired_link_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_expired_link_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    /*
     * set by ngx_pcalloc():
     *
     *     conf->variable = NULL;
     *     conf->md5 = NULL;
     *     conf->secret = { 0, NULL };
     */

    return conf;
}

static ngx_int_t
ngx_http_expired_link_add_variables(ngx_conf_t *cf)
{
    ngx_http_variable_t  *var;

    var = ngx_http_add_variable(cf, &ngx_http_expired_link_name, 0);
    if (var == NULL) {
        return NGX_ERROR;
    }

    var->get_handler = ngx_http_expired_link_variable;

    return NGX_OK;
}
