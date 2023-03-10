/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 2014 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */

/*
* kws_detections.c -- Object for storing keyphrase search results
*/

#include "kws_detections.h"

void
kws_detections_reset(kws_detections_t *detections)
{
    gnode_t *gn;

    if (!detections->detect_list)
        return;

    for (gn = detections->detect_list; gn; gn = gnode_next(gn))
        ckd_free(gnode_ptr(gn));
    glist_free(detections->detect_list);
    detections->detect_list = NULL;
}

void
kws_detections_add(kws_detections_t *detections, const char* keyphrase, int sf, int ef, int prob, int ascr)
{
    gnode_t *gn;
    kws_detection_t* detection;
    for (gn = detections->detect_list; gn; gn = gnode_next(gn)) {
        kws_detection_t *det = (kws_detection_t *)gnode_ptr(gn);
        if (strcmp(keyphrase, det->keyphrase) == 0 && det->sf < ef && det->ef > sf) {
            if (det->prob < prob) {
                det->sf = sf;
                det->ef = ef;
                det->prob = prob;
                det->ascr = ascr;
            }
            return;
        }
    }

    /* Nothing found */
    detection = (kws_detection_t *)ckd_calloc(1, sizeof(*detection));
    detection->sf = sf;
    detection->ef = ef;
    detection->keyphrase = keyphrase;
    detection->prob = prob;
    detection->ascr = ascr;
    detections->detect_list = glist_add_ptr(detections->detect_list, detection);
}

char *
kws_detections_hyp_str(kws_detections_t *detections, int frame, int delay)
{
    gnode_t *gn;
    char *c;
    int len;
    char *hyp_str;

    len = 0;
    for (gn = detections->detect_list; gn; gn = gnode_next(gn)) {
	kws_detection_t *det = (kws_detection_t *)gnode_ptr(gn);
	if (det->ef < frame - delay) {
	    len += strlen(det->keyphrase) + 1;
	}
    }

    if (len == 0) {
        return NULL;
    }

    hyp_str = (char *)ckd_calloc(len, sizeof(char));
    c = hyp_str;
    detections->detect_list = glist_reverse(detections->detect_list);
    for (gn = detections->detect_list; gn; gn = gnode_next(gn)) {
	kws_detection_t *det = (kws_detection_t *)gnode_ptr(gn);
	if (det->ef < frame - delay) {
            memcpy(c, det->keyphrase, strlen(det->keyphrase));
    	    c += strlen(det->keyphrase);
    	    *c = ' ';
    	    c++;
    	}
    }
    if (c > hyp_str) {
        c--;
	*c = '\0';
    }
    detections->detect_list = glist_reverse(detections->detect_list);
    return hyp_str;
}

