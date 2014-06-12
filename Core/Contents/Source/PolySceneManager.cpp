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

#include "PolySceneManager.h"
#include "PolyCamera.h"
#include "PolyCoreServices.h"
#include "PolyLogger.h"
#include "PolyRenderer.h"
#include "PolyScene.h"
#include "PolySceneRenderTexture.h"
#include "PolyTexture.h"

using namespace Polycode;

SceneManager::SceneManager() {
}

SceneManager::~SceneManager() {
	for(int i=0; i < scenes.size(); i++) {	
		delete scenes[i];
	}
	scenes.clear();	
}

void SceneManager::removeScene(Scene *scene) {
	Logger::log("Removing scene\n");
	for(int i=0;i<scenes.size();i++) {
		if(scenes[i] == scene) {
			scenes.erase(scenes.begin()+i);
			return;
		}
	}
}

void SceneManager::registerRenderTexture(SceneRenderTexture *renderTexture) {
	renderTextures.push_back(renderTexture);
}
	
void SceneManager::unregisterRenderTexture(SceneRenderTexture *renderTexture) {
	for(int i=0;i<renderTextures.size();i++) {
		if(renderTextures[i] == renderTexture) {
			renderTextures.erase(renderTextures.begin()+i);
		}
	}	
}
		
void SceneManager::addScene(Scene *newScene) {
	scenes.push_back(newScene);
}

void SceneManager::updateRenderTextures(Scene *scene) {
}

void SceneManager::setRenderer(Renderer *renderer) {
	this->renderer = renderer;
}

void SceneManager::renderVirtual() {
	bool anyVirtualsRendered = false;
	for(int i=0;i<renderTextures.size();i++) {
		if(renderTextures[i]->enabled) {
            renderTextures[i]->Render();
			anyVirtualsRendered = true;
		}			
	}
	renderer->setViewportSize(renderer->getXRes(), renderer->getYRes());
	if (anyVirtualsRendered) {
		renderer->clearScreen();
	}
}

void SceneManager::Render() {
	for(int i=0;i<scenes.size();i++) {
		if(scenes[i]->isEnabled() && !scenes[i]->isVirtual()) {
			renderer->loadIdentity();
			Scene *scene = scenes[i];
			if(scene->getActiveCamera()->hasFilterShader()) {
				scene->getActiveCamera()->drawFilter();
			} else {
				scene->Render();
			}
		}
		renderer->loadIdentity();			
	}
}

void SceneManager::Update() {
	for(int i=0;i<scenes.size();i++) {
		if(scenes[i]->isEnabled()) {
			scenes[i]->Update();
		}
	}
}
