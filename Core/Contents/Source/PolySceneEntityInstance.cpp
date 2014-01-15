/*
 Copyright (C) 2011 by Ivan Safrin
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/

#include "PolySceneEntityInstance.h"
#include "PolyLogger.h"
#include "PolyCoreServices.h"
#include "PolyResourceManager.h"
#include "PolyMaterial.h"
#include "PolySceneLight.h"
#include "PolySceneMesh.h"
#include "PolySceneLabel.h"
#include "PolySceneSound.h"
#include "PolyCamera.h"

using namespace Polycode;

SceneEntityInstanceResourceEntry::SceneEntityInstanceResourceEntry(SceneEntityInstance *instance)  : Resource(Resource::RESOURCE_SCREEN_ENTITY_INSTANCE) {
	this->instance = instance;
}

SceneEntityInstanceResourceEntry::~SceneEntityInstanceResourceEntry() {

}

SceneEntityInstance *SceneEntityInstanceResourceEntry::getInstance() {
	return instance;
}

void SceneEntityInstanceResourceEntry::reloadResource() {
	instance->reloadEntityInstance();
	Resource::reloadResource();
}

SceneEntityInstance *SceneEntityInstance::BlankSceneEntityInstance() {
	return new SceneEntityInstance();
}

SceneEntityInstance::SceneEntityInstance(Scene *parentScene, const String& fileName) : Entity() {
    this->parentScene = parentScene;
	resourceEntry = new SceneEntityInstanceResourceEntry(this);
	loadFromFile(fileName);
	resourceEntry->setResourceName(fileName);
	resourceEntry->setResourcePath(fileName);
	cloneUsingReload = false;
	ownsChildren = true;
    topLevelResourcePool = CoreServices::getInstance()->getResourceManager()->getGlobalPool();
}

SceneEntityInstance::SceneEntityInstance() : Entity() {
	cloneUsingReload = true;
	ownsChildren = true;
	resourceEntry = new SceneEntityInstanceResourceEntry(this);
}

SceneEntityInstance::~SceneEntityInstance() {	
	CoreServices::getInstance()->getResourceManager()->removeResource(resourceEntry);
	delete resourceEntry;
}

void SceneEntityInstance::reloadEntityInstance() {
	loadFromFile(fileName);
}

SceneEntityInstanceResourceEntry *SceneEntityInstance::getResourceEntry() {
	return resourceEntry;
}

Entity *SceneEntityInstance::Clone(bool deepClone, bool ignoreEditorOnly) const {
	SceneEntityInstance *newEntity;
	if(cloneUsingReload) {
		newEntity = new SceneEntityInstance(parentScene, fileName);
	} else {
		newEntity = new SceneEntityInstance();
	}
	applyClone(newEntity, deepClone, ignoreEditorOnly);
	return newEntity;
}

void SceneEntityInstance::applyClone(Entity *clone, bool deepClone, bool ignoreEditorOnly) const {
	if(cloneUsingReload) {
		Entity::applyClone(clone, false, ignoreEditorOnly);
	} else {
		Entity::applyClone(clone, deepClone, ignoreEditorOnly);
		SceneEntityInstance *_clone = (SceneEntityInstance*) clone;
		_clone->fileName = fileName;
	}
}


void SceneEntityInstance::linkResourcePool(ResourcePool *pool) {
    for(int i=0; i < resourcePools.size(); i++) {
        if(resourcePools[i] == pool) {
            return;
        }
    }
    pool->setFallbackPool(topLevelResourcePool);
    topLevelResourcePool = pool;
    resourcePools.push_back(pool);
}

unsigned int SceneEntityInstance::getNumLinkedResourePools() {
    return resourcePools.size();
}

ResourcePool *SceneEntityInstance::getLinkedResourcePoolAtIndex(unsigned int index) {
    return resourcePools[index];
}

void SceneEntityInstance::rebuildResourceLinks() {
    for(int i=0; i < resourcePools.size(); i++) {
        if(i == 0) {
            resourcePools[i]->setFallbackPool(CoreServices::getInstance()->getResourceManager()->getGlobalPool());
        } else {
            resourcePools[i]->setFallbackPool(resourcePools[i-1]);
        }
    }
    
    if(resourcePools.size() > 0) {
        topLevelResourcePool = resourcePools[resourcePools.size()-1];
    } else {
        topLevelResourcePool = CoreServices::getInstance()->getResourceManager()->getGlobalPool();
    }
}

void SceneEntityInstance::unlinkResourcePool(ResourcePool *pool) {
    for(int i=0; i < resourcePools.size(); i++) {
        if(resourcePools[i] == pool) {
            resourcePools.erase(resourcePools.begin() + i);
            rebuildResourceLinks();
            return;
        }
    }
}

void SceneEntityInstance::applySceneMesh(ObjectEntry *entry, SceneMesh *sceneMesh) {
	if(!entry) {
		return;
    }
    
    ObjectEntry *materialName =(*entry)["material"];
    if(materialName) {
        sceneMesh->setMaterialByName(materialName->stringVal, topLevelResourcePool);
        if(sceneMesh->getMaterial()) {
            ObjectEntry *optionsEntry =(*entry)["shader_options"];
            if(optionsEntry) {
                for(int i=0; i < optionsEntry->length; i++) {
                    ObjectEntry *shaderEntry =(*optionsEntry)[i];
                    if(shaderEntry) {
                        
                        // parse in texture bindings
                        ObjectEntry *texturesEntry =(*shaderEntry)["textures"];
                        if(texturesEntry) {
                            for(int j=0; j < texturesEntry->length; j++) {
                                ObjectEntry *textureEntry =(*texturesEntry)[j];
                                if(textureEntry) {
                                    ObjectEntry *nameEntry = (*textureEntry)["name"];
                                    if(nameEntry && textureEntry->stringVal != "") {
                                        
                                        if(textureEntry->name == "cubemap") {
                                            Cubemap *cubemap;
                                            
                                            cubemap = (Cubemap*)topLevelResourcePool->getResource(Resource::RESOURCE_CUBEMAP, textureEntry->stringVal);
                                                
                                                                                      if(cubemap) {
                                                sceneMesh->getLocalShaderOptions()->addCubemap(nameEntry->stringVal, cubemap);
                                            }
                                        } else {
                                            sceneMesh->getLocalShaderOptions()->addTexture(nameEntry->stringVal, CoreServices::getInstance()->getMaterialManager()->createTextureFromFile(textureEntry->stringVal));
                                        }
                                    }
                                }
                            }
                        }
                        
                        ObjectEntry *paramsEntry =(*shaderEntry)["params"];
                        if(paramsEntry) {
                            for(int j=0; j < paramsEntry->length; j++) {
                                ObjectEntry *paramEntry =(*paramsEntry)[j];
                                if(paramEntry) {
                                    ObjectEntry *nameEntry = (*paramEntry)["name"];
                                    ObjectEntry *valueEntry = (*paramEntry)["value"];
                                    if(nameEntry && valueEntry) {
                                        Shader *materialShader = sceneMesh->getMaterial()->getShader(i);
                                        if(materialShader) {
                                            int type = materialShader->getExpectedParamType(nameEntry->stringVal);
                                            LocalShaderParam *param = sceneMesh->getLocalShaderOptions()->addParam(type, nameEntry->stringVal);
                                            if(param) {
                                                param->setParamValueFromString(type, valueEntry->stringVal);
                                            }
                                        }
                                    }
                                    
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
}

void SceneEntityInstance::parseObjectIntoCurve(ObjectEntry *entry, BezierCurve *curve) {
	curve->clearControlPoints();
	ObjectEntry *controlPoints =(*entry)["controlPoints"];
	if(controlPoints) {
		for(int i=0; i < controlPoints->length; i++) {		
			ObjectEntry *controlPoint = ((*controlPoints))[i];		
			if(controlPoint) {
				Vector3 vpt1;
				Vector3 vpt2;
				Vector3 vpt3;
												
				ObjectEntry *pt1 = ((*controlPoint))["pt1"];
				if(pt1) {
					vpt1.x = ((*pt1))["x"]->NumberVal;
					vpt1.y = ((*pt1))["y"]->NumberVal;
					vpt1.z = ((*pt1))["z"]->NumberVal;
				}

				ObjectEntry *pt2 = ((*controlPoint))["pt2"];
				if(pt2) {
					vpt2.x = ((*pt2))["x"]->NumberVal;
					vpt2.y = ((*pt2))["y"]->NumberVal;
					vpt2.z = ((*pt2))["z"]->NumberVal;
					
				}

				ObjectEntry *pt3 = ((*controlPoint))["pt3"];
				if(pt3) {
					vpt3.x = ((*pt3))["x"]->NumberVal;
					vpt3.y = ((*pt3))["y"]->NumberVal;
					vpt3.z = ((*pt3))["z"]->NumberVal;
				}

				curve->addControlPoint(vpt1.x, vpt1.y, vpt1.z, vpt2.x, vpt2.y, vpt2.z, vpt3.x, vpt3.y, vpt3.z);
			}
		}
	}
	
}

Entity *SceneEntityInstance::loadObjectEntryIntoEntity(ObjectEntry *entry, Entity *targetEntity) {

	Entity *entity = NULL;

	ObjectEntry *entityType = (*entry)["type"];
	if(entityType) {
			
        
        /*
         
         if(entityType->stringVal == "SceneEntityInstance") {
         ObjectEntry *screenInstanceEntry = (*entry)["SceneEntityInstance"];
         String filePath = (*screenInstanceEntry)["filePath"]->stringVal;
         SceneEntityInstance *instance = new SceneEntityInstance(filePath);
         entity = instance;
         }
         
         */

		if(entityType->stringVal == "SceneSprite") {
			ObjectEntry *spriteEntry = (*entry)["SceneSprite"];
			String filePath = (*spriteEntry)["filePath"]->stringVal;
			
			SceneSprite *sprite = new SceneSprite(filePath);
			
			String animName = (*spriteEntry)["anim"]->stringVal;
			sprite->playAnimation(animName, -1, false);
			entity = sprite;
            applySceneMesh((*entry)["SceneMesh"], sprite);
        } else 	if(entityType->stringVal == "SceneLabel") {
			ObjectEntry *labelEntry = (*entry)["SceneLabel"];
			
			String text = (*labelEntry)["text"]->stringVal;
			String font = (*labelEntry)["font"]->stringVal;
			int size = (*labelEntry)["size"]->intVal;
            Number actualHeight = (*labelEntry)["actualHeight"]->intVal;
			int aaMode = (*labelEntry)["aaMode"]->intVal;
            
			SceneLabel *label = new SceneLabel(text, size, font, aaMode, actualHeight);
            label->setAnchorPoint(0.0, 0.0, 0.0);
            label->snapToPixels = false;
            label->positionAtBaseline = false;
            applySceneMesh((*entry)["SceneMesh"], label);
            if(label->getLocalShaderOptions()) {
                label->getLocalShaderOptions()->clearTexture("diffuse");
                label->getLocalShaderOptions()->addTexture("diffuse", label->getTexture());
            }
            
			entity = label;
        } else if(entityType->stringVal == "SceneParticleEmitter") {
            
			ObjectEntry *emitterEntry = (*entry)["SceneParticleEmitter"];
            SceneParticleEmitter *emitter = new SceneParticleEmitter(1, 1, 1);

            emitter->setParticleType((*emitterEntry)["type"]->intVal);
            emitter->setParticleCount((*emitterEntry)["count"]->intVal);
            emitter->setParticleLifetime((*emitterEntry)["lifetime"]->NumberVal);
            emitter->setParticleSize((*emitterEntry)["size"]->NumberVal);
            emitter->setParticlesInWorldSpace((*emitterEntry)["world"]->boolVal);
            emitter->setLoopParticles((*emitterEntry)["loop"]->boolVal);
            
            emitter->setParticleRotationSpeed(Vector3((*emitterEntry)["rX"]->NumberVal, (*emitterEntry)["rY"]->NumberVal, (*emitterEntry)["rZ"]->NumberVal));
            emitter->setGravity(Vector3((*emitterEntry)["gX"]->NumberVal, (*emitterEntry)["gY"]->NumberVal, (*emitterEntry)["gZ"]->NumberVal));
            emitter->setParticleDirection(Vector3((*emitterEntry)["dirX"]->NumberVal, (*emitterEntry)["dirY"]->NumberVal, (*emitterEntry)["dirZ"]->NumberVal));
            emitter->setEmitterSize(Vector3((*emitterEntry)["eX"]->NumberVal, (*emitterEntry)["eY"]->NumberVal, (*emitterEntry)["eZ"]->NumberVal));
            emitter->setDirectionDeviation(Vector3((*emitterEntry)["devX"]->NumberVal, (*emitterEntry)["devY"]->NumberVal, (*emitterEntry)["devZ"]->NumberVal));
            
            emitter->setPerlinEnabled((*emitterEntry)["perlin"]->boolVal);
            if(emitter->getPerlinEnabled()) {
                emitter->setPerlinValue(Vector3((*emitterEntry)["pX"]->NumberVal, (*emitterEntry)["pY"]->NumberVal, (*emitterEntry)["pZ"]->NumberVal));
            }
            
            emitter->useColorCurves = (*emitterEntry)["useColorCurves"]->boolVal;
            emitter->useScaleCurve = (*emitterEntry)["useScaleCurve"]->boolVal;
            
            if(emitter->useColorCurves) {
                parseObjectIntoCurve((*emitterEntry)["colorCurveR"], &emitter->colorCurveR);
                parseObjectIntoCurve((*emitterEntry)["colorCurveG"], &emitter->colorCurveG);
                parseObjectIntoCurve((*emitterEntry)["colorCurveB"], &emitter->colorCurveB);
                parseObjectIntoCurve((*emitterEntry)["colorCurveA"], &emitter->colorCurveA);
            }
            
            if(emitter->useScaleCurve) {
                parseObjectIntoCurve((*emitterEntry)["scaleCurve"], &emitter->scaleCurve);
            }
            
            applySceneMesh((*entry)["SceneMesh"], emitter);
			entity = emitter;
            
		} else if(entityType->stringVal == "SceneLight") {
            
			ObjectEntry *lightEntry = (*entry)["SceneLight"];
            if(lightEntry) {
                int lightType = (*lightEntry)["type"]->intVal;
                SceneLight *newLight  = new SceneLight(lightType, parentScene, 0);
                
                newLight->setIntensity((*lightEntry)["intensity"]->NumberVal);
                
                newLight->lightColor.setColor((*lightEntry)["cR"]->NumberVal, (*lightEntry)["cG"]->NumberVal, (*lightEntry)["cB"]->NumberVal, (*lightEntry)["cA"]->NumberVal);
                newLight->specularLightColor.setColor((*lightEntry)["scR"]->NumberVal, (*lightEntry)["scG"]->NumberVal, (*lightEntry)["scB"]->NumberVal, (*lightEntry)["scA"]->NumberVal);

                newLight->setAttenuation((*lightEntry)["cAtt"]->NumberVal, (*lightEntry)["lAtt"]->NumberVal, (*lightEntry)["qAtt"]->NumberVal);
                
                if(newLight->getType() == SceneLight::SPOT_LIGHT) {
                    newLight->setSpotlightProperties((*lightEntry)["spotCutoff"]->NumberVal, (*lightEntry)["spotExponent"]->NumberVal);
                    
                    if((*lightEntry)["shadows"]->boolVal) {
                        newLight->enableShadows(true, (*lightEntry)["shadowmapRes"]->intVal);
                        newLight->setShadowMapFOV((*lightEntry)["shadowmapFOV"]->NumberVal);
                    }
                }
                
                parentScene->addLight(newLight);
                entity = newLight;
            }
 
        } else if(entityType->stringVal == "ScenePrimitive") {
			ObjectEntry *scenePrimitiveEntry = (*entry)["ScenePrimitive"];
			int pType = (*scenePrimitiveEntry)["type"]->intVal;
			Number p1 = (*scenePrimitiveEntry)["p1"]->NumberVal;
			Number p2 = (*scenePrimitiveEntry)["p2"]->NumberVal;
			Number p3 = (*scenePrimitiveEntry)["p3"]->NumberVal;
			Number p4 = (*scenePrimitiveEntry)["p4"]->NumberVal;
			Number p5 = (*scenePrimitiveEntry)["p5"]->NumberVal;
            
			ScenePrimitive *primitive = new ScenePrimitive(pType, p1, p2, p3, p4, p5);
			entity = primitive;
            applySceneMesh((*entry)["SceneMesh"], primitive);
		} else if(entityType->stringVal == "SceneMesh") {
			ObjectEntry *meshEntry = (*entry)["SceneMesh"];
            if(meshEntry) {
                ObjectEntry *fileName = (*meshEntry)["file"];
                if(fileName) {
                    SceneMesh *newMesh = new SceneMesh(fileName->stringVal);
                    applySceneMesh(meshEntry, newMesh);
                    entity = newMesh;
                }
            }
        } else if(entityType->stringVal == "SceneSound") {
			ObjectEntry *soundEntry = (*entry)["SceneSound"];
			
			String filePath = (*soundEntry)["filePath"]->stringVal;
			Number refDistance = (*soundEntry)["refDistance"]->NumberVal;
			Number maxDistance = (*soundEntry)["maxDistance"]->NumberVal;
			Number volume = (*soundEntry)["volume"]->NumberVal;
			Number pitch = (*soundEntry)["pitch"]->NumberVal;
            
			SceneSound *sound = new SceneSound(filePath, refDistance, maxDistance);
			sound->getSound()->setVolume(volume);
			sound->getSound()->setPitch(pitch);
			
			entity = sound;
        } else if(entityType->stringVal == "Camera") {
			ObjectEntry *cameraEntry = (*entry)["Camera"];
            
			Camera *camera = new Camera(parentScene);
            
            camera->setExposureLevel((*cameraEntry)["exposure"]->NumberVal);
            camera->setClippingPlanes((*cameraEntry)["nearClip"]->NumberVal, (*cameraEntry)["farClip"]->NumberVal);
            camera->setOrthoMode((*cameraEntry)["ortho"]->boolVal);
            
            if(camera->getOrthoMode()) {
				camera->setOrthoSizeMode(Camera::ProjectionMode((*cameraEntry)["sizeMode"]->intVal));
                camera->setOrthoSize((*cameraEntry)["orthoWidth"]->NumberVal, (*cameraEntry)["orthoHeight"]->NumberVal);
            } else {
                camera->setFOV((*cameraEntry)["fov"]->NumberVal);
            }
            
			entity = camera;
		}

        
	}

	if(!entity) {
		if(targetEntity) {
			entity = targetEntity;
		} else {
			entity = new Entity();
		}
	}
	
	entity->ownsChildren = true;

	entry->readNumber("bbX", &entity->bBox.x);
	entry->readNumber("bbY", &entity->bBox.y);
	entry->readNumber("bbZ", &entity->bBox.z);

	entity->color.r = (*entry)["cR"]->NumberVal;
	entity->color.g = (*entry)["cG"]->NumberVal;
	entity->color.b = (*entry)["cB"]->NumberVal;
	entity->color.a = (*entry)["cA"]->NumberVal;


	if(!targetEntity) {	
		entity->blendingMode = (*entry)["blendMode"]->intVal;

        entity->setScale((*entry)["sX"]->NumberVal, (*entry)["sY"]->NumberVal, (*entry)["sZ"]->NumberVal);
        entity->setPosition((*entry)["pX"]->NumberVal, (*entry)["pY"]->NumberVal, (*entry)["pZ"]->NumberVal);
        entity->setRotationEuler(Vector3((*entry)["rX"]->NumberVal, (*entry)["rY"]->NumberVal, (*entry)["rZ"]->NumberVal));
	}
	
	if((*entry)["id"]->stringVal != "") {
		entity->id = (*entry)["id"]->stringVal;
	}
	
	String tagString = (*entry)["tags"]->stringVal; 
	
	if(tagString != "") {
		std::vector<String> tags = tagString.split(",");
		for(int i=0; i < tags.size(); i++) {
			entity->addTag(tags[i]);
		}
	}
	
	ObjectEntry *props = (*entry)["props"];
	if(props) {
		for(int i=0; i < props->length; i++) {		
			ObjectEntry *prop = ((*props))[i];		
			if(prop) {
				entity->setEntityProp((*prop)["name"]->stringVal, (*prop)["value"]->stringVal);
			}
		}
	}
														
	ObjectEntry *children = (*entry)["children"];
	
	if(children) {
		for(int i=0; i < children->length; i++) {
			ObjectEntry *childEntry = ((*children))[i];
			ScreenEntity *childEntity = loadObjectEntryIntoEntity(childEntry);
			entity->addChild(childEntity);				
		}
	}

	return entity;
}

String SceneEntityInstance::getFileName() const {
	return fileName;
}

void SceneEntityInstance::clearInstance() {
    
    resourcePools.clear();
    topLevelResourcePool = CoreServices::getInstance()->getResourceManager()->getGlobalPool();
	for(int i=0; i < children.size(); i++) {
		removeChild(children[i]);
		children[i]->setOwnsChildrenRecursive(true);
		delete children[i];
	}
}

bool SceneEntityInstance::loadFromFile(const String& fileName) {

	clearInstance();
	
	resourceEntry->resourceFileTime = OSBasics::getFileTime(fileName);

	this->ownsChildren = true;
	this->fileName = fileName;
	Object loadObject;
	if(!loadObject.loadFromBinary(fileName)) {
        if(!loadObject.loadFromXML(fileName)) {
            Logger::log("Error loading entity instance.\n");
        }
	}
    
	ObjectEntry *settings = loadObject.root["settings"];
    if(settings) {
        ObjectEntry *matFiles = (*settings)["matFiles"];
        for(int i=0; i < matFiles->length; i++) {
            ObjectEntry *matFile = (*matFiles)[i];
            if(matFile) {
                ObjectEntry *path = (*matFile)["path"];
                if(path) {
                    ResourcePool *newPool = new ResourcePool(path->stringVal,  CoreServices::getInstance()->getResourceManager()->getGlobalPool());
                    CoreServices::getInstance()->getMaterialManager()->loadMaterialLibraryIntoPool(newPool, path->stringVal);
                    linkResourcePool(newPool);
                }
            }
        }
    }
    
	ObjectEntry *root = loadObject.root["root"];
	if(root) {
		loadObjectEntryIntoEntity(root, this);
	}
	
	return true;
}
