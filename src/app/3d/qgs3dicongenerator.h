/***************************************************************************
    qgs3dicongenerator.h
    ---------------
    begin                : July 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DICONGENERATOR_H
#define QGS3DICONGENERATOR_H

#include "qgsstylemodel.h"

class QgsSymbol;

class Qgs3DIconGenerator : public QgsAbstractStyleEntityIconGenerator
{
    Q_OBJECT

  public:

    Qgs3DIconGenerator( QObject *parent );

    void generateIcon( QgsStyle *style, QgsStyle::StyleEntity type, const QString &name ) override;

  private:
    struct IconRequest
    {
      IconRequest( QgsStyle *style, QgsStyle::StyleEntity type, const QString &name )
        : style( style )
        , type( type )
        , name( name )
      {}
      QgsStyle *style = nullptr;
      QgsStyle::StyleEntity type = QgsStyle::Symbol3DEntity;
      QString name;

      bool operator==( const IconRequest &other ) const
      {
        return style == other.style && type == other.type && name == other.name;
      }
    };
    QList< IconRequest > mPendingRequests;
    bool mGeneratingIcon = false;

    void generateIcon( const IconRequest &request );
};

#endif //QGS3DICONGENERATOR_H
