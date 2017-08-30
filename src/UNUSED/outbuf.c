
/*************************************************************************
 * Copyright (c) 2017 AT&T Intellectual Property
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: John Ellson <john.ellson@gmail.com>
 *************************************************************************/

success_t ikea_write (ikea_t *ikea, const unsigned char *data, const sixe_t data_len)
{
    if ((bufsx - next) > data_len) {
        memcpy (buf+next, data, data_len);
    }
    else if (next) {
        process (buf, next);
        next=0;
        if (data+len > bufsx) {
            process (data, data_len);
        }
    }
    return SUCCESS;
}

success_t ikea_flush (ikea_t *ikea)
{
    if (next) {
        process (buf, next);
    }
    fflush(fh);
}

success_t ikea_close (ikea_t *ikea)
{
    if (next) {
        process (buf, next);
    }
    fclose(fh);
}
