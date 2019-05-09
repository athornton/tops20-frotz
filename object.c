/* object.c - Object manipulation opcodes
 *	Copyright (c) 1995-1997 Stefan Jokisch
 *
 * This file is part of Frotz.
 *
 * Frotz is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Frotz is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "frotz.h"

#define MAX_OBJECT 2000

#define O1_PARENT 4
#define O1_SIBLING 5
#define O1_CHILD 6
#define O1_PROPERTY_OFFSET 7
#define O1_SIZE 9

#define O4_PARENT 6
#define O4_SIBLING 8
#define O4_CHILD 10
#define O4_PROPERTY_OFFSET 12
#define O4_SIZE 14


/*
 * object_address
 *
 * Calculate the address of an object.
 *
 */
static zword object_address (zword obj)
{
/*    zchar obj_num[10]; */

    /* Check object number */

    if (obj > ((A00035 <= V3) ? 255 : MAX_OBJECT)) {
	A00199("@Attempt to address illegal object ");
	A00197(obj);
	A00199(".  This is normally fatal.");
	A00195();
	A00188 (ERR_ILL_OBJ);
    }

    /* Return object address */

    if (A00035 <= V3)
	return A00041 + ((obj - 1) * O1_SIZE + 62);
    else
	return A00041 + ((obj - 1) * O4_SIZE + 126);

}/* object_address */


/*
 * A00296
 *
 * Return the address of the given object's name.
 *
 */
zword A00296 (zword object)
{
    zword obj_addr;
    zword name_addr;

    obj_addr = object_address (object);

    /* The object name address is found at the start of the properties */

    if (A00035 <= V3)
	obj_addr += O1_PROPERTY_OFFSET;
    else
	obj_addr += O4_PROPERTY_OFFSET;

    LOW_WORD (obj_addr, name_addr)

    return name_addr;

}/* A00296 */


/*
 * first_property
 *
 * Calculate the start address of the property list associated with
 * an object.
 *
 */
static zword first_property (zword obj)
{
    zword prop_addr;
    zbyte size;

    /* Fetch address of object name */

    prop_addr = A00296 (obj);

    /* Get length of object name */

    LOW_BYTE (prop_addr, size)

    /* Add name length to pointer */

    return prop_addr + 1 + 2 * size;

}/* first_property */


/*
 * next_property
 *
 * Calculate the address of the next property in a property list.
 *
 */
static zword next_property (zword prop_addr)
{
    zbyte value;

    /* Load the current property id */

    LOW_BYTE (prop_addr, value)
    prop_addr++;

    /* Calculate the length of this property */

    if (A00035 <= V3)
	value >>= 5;
    else if (!(value & 0x80))
	value >>= 6;
    else {

	LOW_BYTE (prop_addr, value)
	value &= 0x3f;

	if (value == 0) value = 64;	/* demanded by Spec 1.0 */

    }

    /* Add property length to current property pointer */

    return prop_addr + value + 1;

}/* next_property */


/*
 * unlink_object
 *
 * Unlink an object from its parent and siblings.
 *
 */
static void unlink_object (zword object)
{
    zword obj_addr;
    zword parent_addr;
    zword sibling_addr;

    if (object == 0) {
	A00188 (ERR_REMOVE_OBJECT_0);
	return;
    }

    obj_addr = object_address (object);

    if (A00035 <= V3) {

	zbyte parent;
	zbyte younger_sibling;
	zbyte older_sibling;
	zbyte zero = 0;

	/* Get parent of object, and return if no parent */

	obj_addr += O1_PARENT;
	LOW_BYTE (obj_addr, parent)
	if (!parent)
	    return;

	/* Get (older) sibling of object and set both parent and sibling
	   pointers to 0 */

	SET_BYTE (obj_addr, zero)
	obj_addr += O1_SIBLING - O1_PARENT;
	LOW_BYTE (obj_addr, older_sibling)
	SET_BYTE (obj_addr, zero)

	/* Get first child of parent (the youngest sibling of the object) */

	parent_addr = object_address (parent) + O1_CHILD;
	LOW_BYTE (parent_addr, younger_sibling)

	/* Remove object from the list of siblings */

	if (younger_sibling == object)
	    SET_BYTE (parent_addr, older_sibling)
	else {
	    do {
		sibling_addr = object_address (younger_sibling) + O1_SIBLING;
		LOW_BYTE (sibling_addr, younger_sibling)
	    } while (younger_sibling != object);
	    SET_BYTE (sibling_addr, older_sibling)
	}

    } else {

	zword parent;
	zword younger_sibling;
	zword older_sibling;
	zword zero = 0;

	/* Get parent of object, and return if no parent */

	obj_addr += O4_PARENT;
	LOW_WORD (obj_addr, parent)
	if (!parent)
	    return;

	/* Get (older) sibling of object and set both parent and sibling
	   pointers to 0 */

	SET_WORD (obj_addr, zero)
	obj_addr += O4_SIBLING - O4_PARENT;
	LOW_WORD (obj_addr, older_sibling)
	SET_WORD (obj_addr, zero)

	/* Get first child of parent (the youngest sibling of the object) */

	parent_addr = object_address (parent) + O4_CHILD;
	LOW_WORD (parent_addr, younger_sibling)

	/* Remove object from the list of siblings */

	if (younger_sibling == object)
	    SET_WORD (parent_addr, older_sibling)
	else {
	    do {
		sibling_addr = object_address (younger_sibling) + O4_SIBLING;
		LOW_WORD (sibling_addr, younger_sibling)
	    } while (younger_sibling != object);
	    SET_WORD (sibling_addr, older_sibling)
	}

    }

}/* unlink_object */


/*
 * A00102, clear an object attribute.
 *
 *	zargs[0] = object
 *	zargs[1] = number of attribute to be cleared
 *
 */
void A00102 (void)
{
    zword obj_addr;
    zbyte value;

    if (A00075 == SHERLOCK)
	if (zargs[1] == 48)
	    return;

    if (zargs[1] > ((A00035 <= V3) ? 31 : 47))
	A00188 (ERR_ILL_ATTR);

    /* If we are monitoring attribute assignment display a short note */

    if (A00003.attribute_assignment) {
	A00200 ();
	A00199 ("@clear_attr ");
	A00198 (zargs[0]);
	A00199 (" ");
	A00197 (zargs[1]);
	A00201 ();
    }

    if (zargs[0] == 0) {
	A00188 (ERR_CLEAR_ATTR_0);
	return;
    }

    /* Get attribute address */

    obj_addr = object_address (zargs[0]) + zargs[1] / 8;

    /* Clear attribute bit */

    LOW_BYTE (obj_addr, value)
    value &= ~(0x80 >> (zargs[1] & 7));
    SET_BYTE (obj_addr, value)

}/* A00102 */


/*
 * z_jin, A00202 if the first object is inside the second.
 *
 *	zargs[0] = first object
 *	zargs[1] = second object
 *
 */
void z_jin (void)
{
    zword obj_addr;

    /* If we are monitoring object locating display a short note */

    if (A00003.object_locating) {
	A00200 ();
	A00199 ("@jin ");
	A00198 (zargs[0]);
	A00199 (" ");
	A00198 (zargs[1]);
	A00201 ();
    }

    if (zargs[0] == 0) {
	A00188 (ERR_JIN_0);
	A00202 (0 == zargs[1]);
	return;
    }

    obj_addr = object_address (zargs[0]);

    if (A00035 <= V3) {

	zbyte parent;

	/* Get parent id from object */

	obj_addr += O1_PARENT;
	LOW_BYTE (obj_addr, parent)

	/* Branch if the parent is obj2 */

	A00202 (parent == zargs[1]);

    } else {

	zword parent;

	/* Get parent id from object */

	obj_addr += O4_PARENT;
	LOW_WORD (obj_addr, parent)

	/* Branch if the parent is obj2 */

	A00202 (parent == zargs[1]);

    }

}/* z_jin */


/*
 * A00110, store the child of an object.
 *
 *	zargs[0] = object
 *
 */
void A00110 (void)
{
    zword obj_addr;

    /* If we are monitoring object locating display a short note */

    if (A00003.object_locating) {
	A00200 ();
	A00199 ("@get_child ");
	A00198 (zargs[0]);
	A00201 ();
    }

    if (zargs[0] == 0) {
	A00188 (ERR_GET_CHILD_0);
	store (0);
	A00202 (FALSE);
	return;
    }

    obj_addr = object_address (zargs[0]);

    if (A00035 <= V3) {

	zbyte child;

	/* Get child id from object */

	obj_addr += O1_CHILD;
	LOW_BYTE (obj_addr, child)

	/* Store child id and A00202 */

	store (child);
	A00202 (child);

    } else {

	zword child;

	/* Get child id from object */

	obj_addr += O4_CHILD;
	LOW_WORD (obj_addr, child)

	/* Store child id and A00202 */

	store (child);
	A00202 (child);

    }

}/* A00110 */


/*
 * A00112, store the number of the first or next property.
 *
 *	zargs[0] = object
 *	zargs[1] = address of current property (0 gets the first property)
 *
 */
void A00112 (void)
{
    zword prop_addr;
    zbyte value;
    zbyte mask;

    if (zargs[0] == 0) {
	A00188 (ERR_GET_NEXT_PROP_0);
	store (0);
	return;
    }

    /* Property id is in bottom five (six) bits */

    mask = (A00035 <= V3) ? 0x1f : 0x3f;

    /* Load address of first property */

    prop_addr = first_property (zargs[0]);

    if (zargs[1] != 0) {

	/* Scan down the property list */

	do {
	    LOW_BYTE (prop_addr, value)
	    prop_addr = next_property (prop_addr);
	} while ((value & mask) > zargs[1]);

	/* Exit if the property does not exist */

	if ((value & mask) != zargs[1])
	    A00188 (ERR_NO_PROP);

    }

    /* Return the property id */

    LOW_BYTE (prop_addr, value)
    store ((zword) (value & mask));

}/* A00112 */


/*
 * A00113, store the parent of an object.
 *
 *	zargs[0] = object
 *
 */
void A00113 (void)
{
    zword obj_addr;

    /* If we are monitoring object locating display a short note */

    if (A00003.object_locating) {
	A00200 ();
	A00199 ("@get_parent ");
	A00198 (zargs[0]);
	A00201 ();
    }

    if (zargs[0] == 0) {
	A00188 (ERR_GET_PARENT_0);
	store (0);
	return;
    }

    obj_addr = object_address (zargs[0]);

    if (A00035 <= V3) {

	zbyte parent;

	/* Get parent id from object */

	obj_addr += O1_PARENT;
	LOW_BYTE (obj_addr, parent)

	/* Store parent */

	store (parent);

    } else {

	zword parent;

	/* Get parent id from object */

	obj_addr += O4_PARENT;
	LOW_WORD (obj_addr, parent)

	/* Store parent */

	store (parent);

    }

}/* A00113 */


/*
 * A00114, store the value of an object property.
 *
 *	zargs[0] = object
 *	zargs[1] = number of property to be examined
 *
 */
void A00114 (void)
{
    zword prop_addr;
    zword wprop_val;
    zbyte bprop_val;
    zbyte value;
    zbyte mask;

    if (zargs[0] == 0) {
	A00188 (ERR_GET_PROP_0);
	store (0);
	return;
    }

    /* Property id is in bottom five (six) bits */

    mask = (A00035 <= V3) ? 0x1f : 0x3f;

    /* Load address of first property */

    prop_addr = first_property (zargs[0]);

    /* Scan down the property list */

    for (;;) {
	LOW_BYTE (prop_addr, value)
	if ((value & mask) <= zargs[1])
	    break;
	prop_addr = next_property (prop_addr);
    }

    if ((value & mask) == zargs[1]) {	/* property found */

	/* Load property (byte or word sized) */

	prop_addr++;

	if ((A00035 <= V3 && !(value & 0xe0)) || (A00035 >= V4 && !(value & 0xc0))) {

	    LOW_BYTE (prop_addr, bprop_val)
	    wprop_val = bprop_val;

	} else LOW_WORD (prop_addr, wprop_val)

    } else {	/* property not found */

	/* Load default value */

	prop_addr = A00041 + 2 * (zargs[1] - 1);
	LOW_WORD (prop_addr, wprop_val)

    }

    /* Store the property value */

    store (wprop_val);

}/* A00114 */


/*
 * A00115, store the address of an object property.
 *
 *	zargs[0] = object
 *	zargs[1] = number of property to be examined
 *
 */
void A00115 (void)
{
    zword prop_addr;
    zbyte value;
    zbyte mask;

    if (zargs[0] == 0) {
	A00188 (ERR_GET_PROP_ADDR_0);
	store (0);
	return;
    }

    if (A00075 == BEYOND_ZORK)
	if (zargs[0] > MAX_OBJECT)
	    { store (0); return; }

    /* Property id is in bottom five (six) bits */

    mask = (A00035 <= V3) ? 0x1f : 0x3f;

    /* Load address of first property */

    prop_addr = first_property (zargs[0]);

    /* Scan down the property list */

    for (;;) {
	LOW_BYTE (prop_addr, value)
	if ((value & mask) <= zargs[1])
	    break;
	prop_addr = next_property (prop_addr);
    }

    /* Calculate the property address or return zero */

    if ((value & mask) == zargs[1]) {

	if (A00035 >= V4 && (value & 0x80))
	    prop_addr++;
	store ((zword) (prop_addr + 1));

    } else store (0);

}/* A00115 */


/*
 * A00116, store the length of an object property.
 *
 * 	zargs[0] = address of property to be examined
 *
 */
void A00116 (void)
{
    zword addr;
    zbyte value;

    /* Back up the property pointer to the property id */

    addr = zargs[0] - 1;
    LOW_BYTE (addr, value)

    /* Calculate length of property */

    if (A00035 <= V3)
	value = (value >> 5) + 1;
    else if (!(value & 0x80))
	value = (value >> 6) + 1;
    else {

	value &= 0x3f;

	if (value == 0) value = 64;	/* demanded by Spec 1.0 */

    }

    /* Store length of property */

    store (value);

}/* A00116 */


/*
 * A00117, store the sibling of an object.
 *
 *	zargs[0] = object
 *
 */
void A00117 (void)
{
    zword obj_addr;

    if (zargs[0] == 0) {
	A00188 (ERR_GET_SIBLING_0);
	store (0);
	A00202 (FALSE);
	return;
    }

    obj_addr = object_address (zargs[0]);

    if (A00035 <= V3) {

	zbyte sibling;

	/* Get sibling id from object */

	obj_addr += O1_SIBLING;
	LOW_BYTE (obj_addr, sibling)

	/* Store sibling and A00202 */

	store (sibling);
	A00202 (sibling);

    } else {

	zword sibling;

	/* Get sibling id from object */

	obj_addr += O4_SIBLING;
	LOW_WORD (obj_addr, sibling)

	/* Store sibling and A00202 */

	store (sibling);
	A00202 (sibling);

    }

}/* A00117 */


/*
 * A00121, make an object the first child of another object.
 *
 *	zargs[0] = object to be moved
 *	zargs[1] = destination object
 *
 */
void A00121 (void)
{
    zword obj1 = zargs[0];
    zword obj2 = zargs[1];
    zword obj1_addr;
    zword obj2_addr;

    /* If we are monitoring object movements display a short note */

    if (A00003.object_movement) {
	A00200 ();
	A00199 ("@move_obj ");
	A00198 (obj1);
	A00199 (" ");
	A00198 (obj2);
	A00201 ();
    }

    if (obj1 == 0) {
	A00188 (ERR_MOVE_OBJECT_0);
	return;
    }

    if (obj2 == 0) {
	A00188 (ERR_MOVE_OBJECT_TO_0);
	return;
    }

    /* Get addresses of both objects */

    obj1_addr = object_address (obj1);
    obj2_addr = object_address (obj2);

    /* Remove object 1 from current parent */

    unlink_object (obj1);

    /* Make object 1 first child of object 2 */

    if (A00035 <= V3) {

	zbyte child;

	obj1_addr += O1_PARENT;
	SET_BYTE (obj1_addr, obj2)
	obj2_addr += O1_CHILD;
	LOW_BYTE (obj2_addr, child)
	SET_BYTE (obj2_addr, obj1)
	obj1_addr += O1_SIBLING - O1_PARENT;
	SET_BYTE (obj1_addr, child)

    } else {

	zword child;

	obj1_addr += O4_PARENT;
	SET_WORD (obj1_addr, obj2)
	obj2_addr += O4_CHILD;
	LOW_WORD (obj2_addr, child)
	SET_WORD (obj2_addr, obj1)
	obj1_addr += O4_SIBLING - O4_PARENT;
	SET_WORD (obj1_addr, child)

    }

}/* A00121 */


/*
 * A00149, set the value of an object property.
 *
 *	zargs[0] = object
 *	zargs[1] = number of property to set
 *	zargs[2] = value to set property to
 *
 */
void A00149 (void)
{
    zword prop_addr;
    zword value;
    zbyte mask;

    if (zargs[0] == 0) {
	A00188 (ERR_PUT_PROP_0);
	return;
    }

    /* Property id is in bottom five or six bits */

    mask = (A00035 <= V3) ? 0x1f : 0x3f;

    /* Load address of first property */

    prop_addr = first_property (zargs[0]);

    /* Scan down the property list */

    for (;;) {
	LOW_BYTE (prop_addr, value)
	if ((value & mask) <= zargs[1])
	    break;
	prop_addr = next_property (prop_addr);
    }

    /* Exit if the property does not exist */

    if ((value & mask) != zargs[1])
	A00188 (ERR_NO_PROP);

    /* Store the new property value (byte or word sized) */

    prop_addr++;

    if ((A00035 <= V3 && !(value & 0xe0)) || (A00035 >= V4 && !(value & 0xc0))) {
	zbyte v = zargs[2];
	SET_BYTE (prop_addr, v)
    } else {
	zword v = zargs[2];
	SET_WORD (prop_addr, v)
    }

}/* A00149 */


/*
 * A00156, unlink an object from its parent and siblings.
 *
 *	zargs[0] = object
 *
 */
void A00156 (void)
{
    /* If we are monitoring object movements display a short note */

    if (A00003.object_movement) {
	A00200 ();
	A00199 ("@remove_obj ");
	A00198 (zargs[0]);
	A00201 ();
    }

    /* Call unlink_object to do the job */

    unlink_object (zargs[0]);

}/* A00156 */


/*
 * A00167, set an object attribute.
 *
 *	zargs[0] = object
 *	zargs[1] = number of attribute to set
 *
 */
void A00167 (void)
{
    zword obj_addr;
    zbyte value;

    if (A00075 == SHERLOCK)
	if (zargs[1] == 48)
	    return;

    if (zargs[1] > ((A00035 <= V3) ? 31 : 47))
	A00188 (ERR_ILL_ATTR);

    /* If we are monitoring attribute assignment display a short note */

    if (A00003.attribute_assignment) {
	A00200 ();
	A00199 ("@set_attr ");
	A00198 (zargs[0]);
	A00199 (" ");
	A00197 (zargs[1]);
	A00201 ();
    }

    if (zargs[0] == 0) {
	A00188 (ERR_SET_ATTR_0);
	return;
    }

    /* Get attribute address */

    obj_addr = object_address (zargs[0]) + zargs[1] / 8;

    /* Load attribute byte */

    LOW_BYTE (obj_addr, value)

    /* Set attribute bit */

    value |= 0x80 >> (zargs[1] & 7);

    /* Store attribute byte */

    SET_BYTE (obj_addr, value)

}/* A00167 */


/*
 * A00181, A00202 if an object attribute is set.
 *
 *	zargs[0] = object
 *	zargs[1] = number of attribute to test
 *
 */
void A00181 (void)
{
    zword obj_addr;
    zbyte value;

    if (zargs[1] > ((A00035 <= V3) ? 31 : 47))
	A00188 (ERR_ILL_ATTR);

    /* If we are monitoring attribute testing display a short note */

    if (A00003.attribute_testing) {
	A00200 ();
	A00199 ("@test_attr ");
	A00198 (zargs[0]);
	A00199 (" ");
	A00197 (zargs[1]);
	A00201 ();
    }

    if (zargs[0] == 0) {
	A00188 (ERR_TEST_ATTR_0);
	A00202 (FALSE);
	return;
    }

    /* Get attribute address */

    obj_addr = object_address (zargs[0]) + zargs[1] / 8;

    /* Load attribute byte */

    LOW_BYTE (obj_addr, value)

    /* Test attribute */

    A00202 (value & (0x80 >> (zargs[1] & 7)));

}/* A00181 */
