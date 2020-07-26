/***************************************************************************
    qgs3dicongenerator.cpp
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

#include "qgs3dicongenerator.h"
#include "qgsapplication.h"
#include "qgsabstract3dsymbol.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayer3drenderer.h"
#include "qgslinestring.h"
#include "qgs3dmapsettings.h"
#include "qgsflatterraingenerator.h"
#include "qgsoffscreen3dengine.h"
#include "qgs3dutils.h"
#include "qgs3dmapscene.h"
#include "qgscameracontroller.h"

Qgs3DIconGenerator::Qgs3DIconGenerator( QObject *parent )
  : QgsAbstractStyleEntityIconGenerator( parent )
{

}

void Qgs3DIconGenerator::generateIcon( QgsStyle *style, QgsStyle::StyleEntity type, const QString &name )
{
  if ( type != QgsStyle::Symbol3DEntity )
    return;

  std::unique_ptr< QgsAbstract3DSymbol > symbol( style->symbol3D( name ) );

  QgsRectangle fullExtent( 0, 0, 1000, 1000 );

  std::unique_ptr< QgsVectorLayer > tempLayer;

  if ( symbol->type() == QStringLiteral( "lines" ) )
  {
    tempLayer = qgis::make_unique< QgsVectorLayer>( "LineString?crs=EPSG:27700", "lines", "memory" );
    QVector<QgsPoint> pts;
    pts << QgsPoint( 0, 0, 10 ) << QgsPoint( 0, 1000, 10 ) << QgsPoint( 1000, 1000, 10 ) << QgsPoint( 1000, 0, 10 );
    pts << QgsPoint( 1000, 0, 500 ) << QgsPoint( 1000, 1000, 500 ) << QgsPoint( 0, 1000, 500 ) << QgsPoint( 0, 0, 500 );
    QgsFeature f1( tempLayer->fields() );
    f1.setGeometry( QgsGeometry( new QgsLineString( pts ) ) );
    QgsFeatureList flist;
    flist << f1;
    tempLayer->dataProvider()->addFeatures( flist );
  }
  else if ( symbol->type() == QStringLiteral( "point" ) )
  {
    tempLayer = qgis::make_unique< QgsVectorLayer>( "PointZ?crs=EPSG:27700", "lines", "memory" );
    QgsFeature f1( tempLayer->fields() );
    f1.setGeometry( QgsGeometry( new QgsPoint( 500, 500, 200 ) ) );
    QgsFeatureList flist;
    flist << f1;
    tempLayer->dataProvider()->addFeatures( flist );
  }
  else
    return;

  tempLayer->setRenderer3D( new QgsVectorLayer3DRenderer( symbol.release() ) );


  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( tempLayer->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << tempLayer.get() );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs() );
  flatTerrain->setExtent( fullExtent );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine engine;
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, &engine );
  engine.setRootEntity( scene );

  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 45, 45 );

  QImage img = Qgs3DUtils::captureSceneImage( engine, scene );

  QIcon icon;
  const QList< QSize > sizes = iconSizes();
  if ( sizes.isEmpty() )
    icon.addFile( QgsApplication::defaultThemePath() + QDir::separator() + QStringLiteral( "3d.svg" ), QSize( 24, 24 ) );
  for ( const QSize &s : sizes )
  {
    QImage scaled = img.scaled( s.width(), s.height() );
    icon.addPixmap( QPixmap::fromImage( scaled ) );
//    icon.addFile( QgsApplication::defaultThemePath() + QDir::separator() + QStringLiteral( "3d.svg" ), s );
  }

  emit iconGenerated( type, name, icon );
}
