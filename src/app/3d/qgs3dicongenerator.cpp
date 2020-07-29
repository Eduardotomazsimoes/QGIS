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
#include "qgsmultipolygon.h"
#include "qgspolygon.h"

Qgs3DIconGenerator::Qgs3DIconGenerator( QObject *parent )
  : QgsAbstractStyleEntityIconGenerator( parent )
{

}

void Qgs3DIconGenerator::generateIcon( QgsStyle *style, QgsStyle::StyleEntity type, const QString &name )
{
  if ( type != QgsStyle::Symbol3DEntity )
    return;

  IconRequest request( style, type, name );
  if ( mPendingRequests.contains( request ) )
    return;

  mPendingRequests.append( request );

  if ( mGeneratingIcon )
    return;

  mGeneratingIcon = true;
  generateIcon( mPendingRequests.first() );
}

void Qgs3DIconGenerator::generateIcon( const Qgs3DIconGenerator::IconRequest &request )
{
  std::unique_ptr< QgsAbstract3DSymbol > symbol( request.style->symbol3D( request.name ) );

  QgsRectangle fullExtent( 0, 0, 1000, 1000 );

  QgsVectorLayer *tempLayer = nullptr;

  if ( symbol->type() == QStringLiteral( "line" ) )
  {
    tempLayer = new QgsVectorLayer( "LineString?crs=EPSG:27700", "lines", "memory" );
    QVector<QgsPoint> pts;
    pts << QgsPoint( 0, 0, 10 ) << QgsPoint( 0, 1000, 10 ) << QgsPoint( 1000, 1000, 10 ) << QgsPoint( 1000, 0, 10 );
    pts << QgsPoint( 1000, 0, 500 ) << QgsPoint( 1000, 1000, 500 ) << QgsPoint( 0, 1000, 500 ) << QgsPoint( 0, 0, 500 );
    QgsFeature f1( tempLayer->fields() );
    f1.setGeometry( QgsGeometry( new QgsLineString( pts ) ) );
    QgsFeatureList flist;
    flist << f1;
    tempLayer->dataProvider()->addFeatures( flist );
  }
  else if ( symbol->type() == QStringLiteral( "polygon" ) )
  {
    tempLayer = new QgsVectorLayer( "MultiPolygonZ?crs=EPSG:27700", "polygons", "memory" );

    QgsMultiPolygon mp;
    mp.addGeometry( new QgsPolygon( new QgsLineString( QVector< QgsPoint >() << QgsPoint( 0, 0, 0 )
                                    << QgsPoint( 0, 1000, 0 )
                                    << QgsPoint( 1000, 1000, 0 )
                                    << QgsPoint( 1000, 0, 0 )
                                    << QgsPoint( 0, 0, 0 ) ) ) );
    mp.addGeometry( new QgsPolygon( new QgsLineString( QVector< QgsPoint >() << QgsPoint( 0, 0, 0 )
                                    << QgsPoint( 0, 1000, 0 )
                                    << QgsPoint( 0, 1000, 500 )
                                    << QgsPoint( 0, 0, 500 )
                                    << QgsPoint( 0, 0, 0 ) ) ) );
    mp.addGeometry( new QgsPolygon( new QgsLineString( QVector< QgsPoint >() << QgsPoint( 0, 0, 0 )
                                    << QgsPoint( 1000, 0, 0 )
                                    << QgsPoint( 1000, 0, 500 )
                                    << QgsPoint( 0, 0, 500 )
                                    << QgsPoint( 0, 0, 0 ) ) ) );
    mp.addGeometry( new QgsPolygon( new QgsLineString( QVector< QgsPoint >() << QgsPoint( 0, 1000, 0 )
                                    << QgsPoint( 1000, 1000, 0 )
                                    << QgsPoint( 1000, 1000, 500 )
                                    << QgsPoint( 0, 1000, 500 )
                                    << QgsPoint( 0, 1000, 0 ) ) ) );
    mp.addGeometry( new QgsPolygon( new QgsLineString( QVector< QgsPoint >() << QgsPoint( 1000, 0, 0 )
                                    << QgsPoint( 1000, 1000, 0 )
                                    << QgsPoint( 1000, 1000, 500 )
                                    << QgsPoint( 1000, 0, 500 )
                                    << QgsPoint( 1000, 0, 0 ) ) ) );
    mp.addGeometry( new QgsPolygon( new QgsLineString( QVector< QgsPoint >() << QgsPoint( 0, 0, 500 )
                                    << QgsPoint( 0, 1000, 500 )
                                    << QgsPoint( 1000, 1000, 500 )
                                    << QgsPoint( 1000, 0, 500 )
                                    << QgsPoint( 0, 0, 500 ) ) ) );

    QgsFeature f1( tempLayer->fields() );
    f1.setGeometry( QgsGeometry( mp.clone() ) );
    QgsFeatureList flist;
    flist << f1;
    tempLayer->dataProvider()->addFeatures( flist );
  }
  else if ( symbol->type() == QStringLiteral( "point" ) )
  {
    tempLayer = new QgsVectorLayer( "PointZ?crs=EPSG:27700", "lines", "memory" );
    QgsFeature f1( tempLayer->fields() );
    f1.setGeometry( QgsGeometry( new QgsPoint( 500, 500, 200 ) ) );
    QgsFeatureList flist;
    flist << f1;
    tempLayer->dataProvider()->addFeatures( flist );
  }
  else
  {
    mPendingRequests.removeAll( request );
    if ( mPendingRequests.empty() )
      mGeneratingIcon = false;
    else
      generateIcon( mPendingRequests.first() );
    return;
  }

  tempLayer->setRenderer3D( new QgsVectorLayer3DRenderer( symbol.release() ) );

  Qgs3DMapSettings *map = new Qgs3DMapSettings;
  map->setCrs( tempLayer->crs() );
  map->setOrigin( QgsVector3D( fullExtent.center().x(), fullExtent.center().y(), 0 ) );
  map->setLayers( QList<QgsMapLayer *>() << tempLayer );
  QgsPointLightSettings light;
  light.setPosition( QVector3D( 1500, 1000, 1300 ) );
  map->setPointLights( QList< QgsPointLightSettings >() << light );

  QgsFlatTerrainGenerator *flatTerrain = new QgsFlatTerrainGenerator;
  flatTerrain->setCrs( map->crs() );
  flatTerrain->setExtent( fullExtent );
  map->setTerrainGenerator( flatTerrain );

  QgsOffscreen3DEngine *engine = new QgsOffscreen3DEngine();
  Qgs3DMapScene *scene = new Qgs3DMapScene( *map, engine );
  engine->setRootEntity( scene );
  scene->cameraController()->setLookingAtPoint( QgsVector3D( 0, 0, 0 ), 2500, 45, 45 );
  engine->setClearColor( QColor( 0, 100, 0 ) );

  const QList< QSize > sizes = iconSizes();
  if ( sizes.isEmpty() )
    engine->setSize( QSize( 24, 24 ) );
  else
  {
    QSize largest( 0, 0 );
    for ( const QSize size : sizes )
    {
      if ( size.width() > largest.width() )
        largest.setWidth( size.width() );
      if ( size.height() > largest.height() )
        largest.setHeight( size.height() );
    }
    engine->setSize( QSize( largest.width() * 3, largest.height() * 3 ) );
  }

  auto requestImageFcn = [engine, scene]
  {
    if ( scene->sceneState() == Qgs3DMapScene::Ready )
    {
      engine->requestCaptureImage();
    }
  };

  auto saveImageFcn = [this, scene, engine, map, request, tempLayer]( const QImage & img )
  {
    scene->deleteLater();
    engine->deleteLater();
    tempLayer->deleteLater();
    delete map;

    QIcon icon;
    const QList< QSize > sizes = iconSizes();
    if ( sizes.isEmpty() )
    {
      QImage scaled = img.scaled( 24, 24 );
      icon.addPixmap( QPixmap::fromImage( scaled ) );
    }
    else
    {
      for ( const QSize &s : sizes )
      {
        QImage scaled( s, QImage::Format_ARGB32 );
        if ( s.width() > 30 )
        {
          scaled.fill( Qt::transparent );
          QPainter p( &scaled );
          p.setBrush( QBrush( Qt::black ) );
          p.setPen( Qt::NoPen );
          p.setRenderHint( QPainter::Antialiasing, true );
          p.drawRoundedRect( scaled.rect(), 6, 6 );
          p.end();
        }
        else
        {
          scaled.fill( Qt::black );
        }

        QPainter p( &scaled );
        p.setCompositionMode( QPainter::CompositionMode_SourceIn );
        p.drawImage( scaled.rect(), img, img.rect() );
        p.end();

        icon.addPixmap( QPixmap::fromImage( scaled ) );
      }
    }

    mPendingRequests.removeAll( request );
    emit iconGenerated( request.type, request.name, icon );

    if ( mPendingRequests.empty() )
      mGeneratingIcon = false;
    else
      generateIcon( mPendingRequests.first() );
  };

  QObject::connect( engine, &QgsAbstract3DEngine::imageCaptured, saveImageFcn );
  if ( scene->sceneState() == Qgs3DMapScene::Ready )
  {
    requestImageFcn();
  }
  else
  {
    // first wait until scene is loaded
    QObject::connect( scene, &Qgs3DMapScene::sceneStateChanged, requestImageFcn );
  }
}
