/* MirroredTypeException.java -- An attempt to access a TypeMirror's class.
   Copyright (C) 2012  Free Software Foundation, Inc.

This file is part of GNU Classpath.

GNU Classpath is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Classpath is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Classpath; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301 USA.

Linking this library statically or dynamically with other modules is
making a combined work based on this library.  Thus, the terms and
conditions of the GNU General Public License cover the whole
combination.

As a special exception, the copyright holders of this library give you
permission to link this library with independent modules to produce an
executable, regardless of the license terms of these independent
modules, and to copy and distribute the resulting executable under
terms of your choice, provided that you also meet, for each linked
independent module, the terms and conditions of the license of that
module.  An independent module is a module which is not derived from
or based on this library.  If you modify this library, you may extend
this exception to your version of the library, but you are not
obligated to do so.  If you do not wish to do so, delete this
exception statement from your version. */

package javax.lang.model.type;

/**
 * Thrown when an application attempts to access the
 * {@link Class} object corresponding to a {@link TypeMirror}.
 *
 * @author Andrew John Hughes (gnu_andrew@member.fsf.org)
 * @since 1.6
 * @see javax.lang.model.element.Element#getAnnotation(Class)
 */
public class MirroredTypeException
  extends RuntimeException
{

  private static final long serialVersionUID = 269L;

  /**
   * The type mirror.
   */
  private TypeMirror mirror;

  /**
   * Constructs a new {@code MirroredTypeException}
   * for the specified type.
   *
   * @param mirror the mirrored type accessed.  May be
   *               {@code null}.
   */
  public MirroredTypeException(TypeMirror mirror)
  {
    this.mirror = mirror;
  }

  /**
   * Returns the type mirror or {@code null}
   * if unavailable.  The type mirror may be {@code null} if
   * the type mirror is not {@link java.io.Serializable} but the
   * exception has been serialized and read back in.
   *
   * @return the type mirror.
   */
  public TypeMirror getTypeMirror()
  {
    return mirror;
  }


}
