/*
 * libwebsockets - small server side websockets and web server implementation
 *
 * Copyright (C) 2010-2018 Andy Green <andy@warmcat.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation:
 *  version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */

#include <private-libwebsockets.h>

static int
rops_handle_POLLIN_cgi(struct lws_context_per_thread *pt, struct lws *wsi,
		       struct lws_pollfd *pollfd)
{
	struct lws_cgi_args args;

	assert(lwsi_role(wsi) == LWSI_ROLE_CGI);

	if (wsi->cgi_channel >= LWS_STDOUT &&
	    !(pollfd->revents & pollfd->events & LWS_POLLIN))
		return LWS_HPI_RET_HANDLED;

	if (wsi->cgi_channel == LWS_STDIN &&
	    !(pollfd->revents & pollfd->events & LWS_POLLOUT))
		return LWS_HPI_RET_HANDLED;

	if (wsi->cgi_channel == LWS_STDIN &&
	    lws_change_pollfd(wsi, LWS_POLLOUT, 0)) {
		lwsl_info("failed at set pollfd\n");
		return LWS_HPI_RET_DIE;
	}

	args.ch = wsi->cgi_channel;
	args.stdwsi = &wsi->parent->cgi->stdwsi[0];
	args.hdr_state = wsi->hdr_state;

	lwsl_debug("CGI LWS_STDOUT %p wsistate 0x%x\n",
		   wsi->parent, wsi->wsistate);

	if (user_callback_handle_rxflow(wsi->parent->protocol->callback,
					wsi->parent, LWS_CALLBACK_CGI,
					wsi->parent->user_space,
					(void *)&args, 0))
		return 1;

	return LWS_HPI_RET_HANDLED;
}

static int
rops_handle_POLLOUT_cgi(struct lws *wsi)
{
	return LWS_HP_RET_USER_SERVICE;
}

static int
rops_periodic_checks_cgi(struct lws_context *context, int tsi, time_t now)
{
	struct lws_context_per_thread *pt = &context->pt[tsi];

	lws_cgi_kill_terminated(pt);

	return 0;
}

struct lws_role_ops role_ops_cgi = {
	"cgi",
	rops_handle_POLLIN_cgi,
	rops_handle_POLLOUT_cgi,
	rops_periodic_checks_cgi,
	NULL,
	NULL,
	NULL,
	NULL
};