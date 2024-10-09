#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>
#include "SFML/Graphics.hpp"
#include "TGUI/TGUI.hpp"

using namespace std;


//################### INFO ####################

#pragma region Description

/*
Project: AY 101 (Dr. Balin) Creative Project
Created: 18 MAR 2021
Updated: 9 OCT 2024
Author: Samuel Daly


Note:
	As it turns out, this is not a remotely accurate simulation, however it is still an interesting project (to me anyway)
	and has some neat math, like using parametric equations for elliptical motion. It was for an intro astronomy class
	in which we had to make a creative project. I believe the mandate we were given was that it be related to our hobbies,
	or that it be some piece of "Haute Couture". This is quite possibly the first complete and functional project I ever made.
	-Sam Daly, 9 OCT 2024


---------------------------------------------------------------------------------------------------------------------------------------------

This Project is a simple simulation of orbital motion according to Kepler's laws
Keplers three laws are:
# every planet's orbit is an ellipse with the Sun at a focus
# a line joining the Sun and a planet sweeps out equal areas in equal times
# orbital period increases as distance from the sun increases (t^2 = a^3)

The first law will be demonsrated visually, the user should be able to see by simply
looking that the motion is elliptical and that the sun is a focus rather than a locus.
To this end the planet graphics will have an ellipse representing its orbit when selected.

The second law will be demonstrated via [INSERT MO]


	Current Bugs
	--------------
	-Cant select while zooming
	-Moons dont orbit
	

	Fixed Bugs (Hopefully)
	----------------
	-Movement speeds up when mouse moved, probably due to inacuraccy in sf::clock
		+Added an Sf time var to store an elapsed time to use for delta time
	-Orbiting planets do not resume in the same position after an unpause
		+Removed redundant pause logic that caused the function to not run, and thus delta time to not update
	-Orbits not centered (Fixed visually, but not mathmatically)
		+Distance equation was flawed, corrected it
	-Selected planet values dont update in real time
		+Moved selected values update to general update loop


	To do
	-----------
	-Add acceleration and velocity calculations
	-UI
	-Figure out how to relate keplers laws
		+Text Description?
		+Sweeping Lines?
	-Standardize distances and scales
	-Orbit lines?
	-Deacllocate dynamic stuff

Distance 1px = 1,000,000 km
Velocity True Scale, corrected in post
Mass  divide by 10


*/

#pragma endregion

//################ CONSTANTS ################
#define PI          3.14159265358979323846
#define G			6.674e-11
#define RADEG       (180.0/PI)
#define DEGRAD      (PI/180.0)
#define sind(x)     sin((x)*DEGRAD)
#define cosd(x)     cos((x)*DEGRAD)
#define tand(x)     tan((x)*DEGRAD)
#define asind(x)    (RADEG*asin(x))
#define acosd(x)    (RADEG*acos(x))
#define atand(x)    (RADEG*atan(x))
#define atan2d(y,x) (RADEG*atan2((y),(x)))


//################ FUNCTION DECS ################
sf::Vector2f paraPos(float h, float k, float t, float a, float b);

float getDist(sf::Vector2f a, sf::Vector2f b);

 sf::Vector2f getMidP(sf::Vector2f a, sf::Vector2f b);

float triArea(sf::Vector2f a, sf::Vector2f b, sf::Vector2f sunPos);

void initGui(tgui::Gui* targ);

void pause(bool isP);

//################ CLASSES ###################

class Nbody {
private:
	//Data
	Nbody* parent = nullptr;
	std::vector<Nbody*> children;
	sf::Clock orbitClock;

	float sMajorAxis = 0.0;
	float sMinorAxis = 0.0;
	float eTricity = 0.0;
	float velocity = 0.0;
	float uAngle = 0.0;
	float c = 0.0;
	float frac = 0.f;
	float mass = 1.f;

	int numOrbits = 0;

	sf::Vector2f orbitCent;

	sf::CircleShape graphic;
	sf::Vertex orbitLine[2] = {sf::Vertex(sf::Vector2f(0,0)), sf::Vertex(sf::Vector2f(0,0))};
	

	std::string name = "Default_Name";
	int type = 0;


public:
	//Con and De Structor
	Nbody(std::string name, int typ, float size, float orbitDist, float v, float e, float m, float initX, float initY) {
		//Set paramaters
		this->orbitCent.x, this->orbitCent.y = 0;
		this->name = name;
		this->graphic.setRadius(size);
		this->velocity = v;
		this->sMajorAxis = orbitDist;
		this->eTricity = e;
		this->mass = m;
		this->type = typ;

		this->graphic.setPosition(initX, initY);

		//Calculate paramaters
		this->sMinorAxis = (this->sMajorAxis * sqrt(1 - pow(this->eTricity, 2)));
		this->c = sqrt(pow(this->sMajorAxis, 2) - pow(this->sMinorAxis, 2));
		 

		//Cosmetics
		if (this->type == 0) {
			this->graphic.setFillColor(sf::Color::Green);
		}
		else if (this->type == 1) {
			this->graphic.setFillColor(sf::Color::Red);
		}
		else {
			this->graphic.setFillColor(sf::Color::Yellow);
		}
		
	}
	~Nbody() {

	}
	 
	//Modifier Functions
	void orbit(bool pause) {
		
		if (!pause) {
			if (this->parent->parent != nullptr) {
				//Account for moving parents
				this->orbitCent.x = this->parent->getCenter().x + c;
				this->orbitCent.y = this->parent->getCenter().y;
				this->velocity = sqrt((G * this->parent->mass) / pow((this->getParDist() * 1000000), 2));
			}
			if (this->parent == nullptr) {
				//If there is no parent from this body then it cannot orbit
				std::cout << "Cannot commence orbit, invalid parent" << std::endl;

			}
			else
			{
				//If this body does orbit a parent then orbit accordingly
				if (this->uAngle >= 2 * PI) {
					this->numOrbits++;
					this->uAngle = 0.0;

				}
				else {
					this->uAngle += this->orbitClock.getElapsedTime().asSeconds() * (this->velocity / 50); //Tweak this formula as needed
				}

				//Update Velocity
				this->velocity = sqrt((G*this->parent->mass) / pow((this->getParDist() * 1000000), 2));
				
			}

			//Update position 
			this->graphic.setPosition(paraPos(this->orbitCent.x, this->orbitCent.y, this->uAngle, this->sMajorAxis, this->sMinorAxis));
			this->orbitLine[0] = this->parent->getCenter();
			this->orbitLine[1] = this->getCenter();
			this->orbitLine->color = sf::Color::Blue;

		}
		
		this->orbitClock.restart();
		
	}

	void setParent(Nbody* targetP) {
	
		//Update Pointers for child and parent
		this->parent = targetP;
		targetP->children.push_back(this);

		//Update position of graphic
		this->graphic.setPosition(targetP->graphic.getPosition().x - this->sMajorAxis, targetP->graphic.getPosition().y);

		//Update the center of the orbit (parent is left / top focus)

		this->orbitCent.x = targetP->getCenter().x + this->c;
		this->orbitCent.y = targetP->getCenter().y;
		
	}

	void addChild(Nbody* target) {
		target->setParent(this);
	}
	
	void draw(sf::RenderWindow* w) {
		if (this != nullptr) {
			//Lines
			w->draw(this->orbitLine, 2, sf::Lines);
		}
		
	}

	//Accessor Functions

	sf::Vector2f getCenter() {
		//This function exists to resolve any discrepancy between
		//the coordinates of the circleShape and the coordinates of its visual center
		//It may not need to exist if the method for positioning and origin setting is modified
		sf::Vector2f temp(0, 0);

		if (this == nullptr) {
			temp.x = 0;
				temp.y = 0;
		}
		else {
			temp.x = this->graphic.getPosition().x + this->graphic.getRadius();
			temp.y = this->graphic.getPosition().y + this->graphic.getRadius();
		}

		return temp;
	}

	sf::CircleShape* getGraphic() {
		return &this->graphic;
	}
	
	float getComp() {
		return this->uAngle;
	}

	int getNumOrbits() {
		return this->numOrbits;
	}

	float getParDist() {
		//Get the distance of this body from its parent

		if (this == nullptr) {
			return -1.f;
		}
		else if (this->parent == nullptr) { 
			return -1.f;
		}else {
			float temp = sqrt(pow(this->getCenter().x - this->parent->getCenter().x, 2) + pow(this->getCenter().y - this->parent->getCenter().y, 2));
			return temp;
		}

	}

	Nbody* getParent() {
		if (this->parent == nullptr || this == nullptr) {
			return nullptr;
		}
		else {
			return this->parent;
		}
	}

	float getVel() {
		if (this == nullptr) {
			return 0;
		}
		else {
			return this->velocity;
		}

	}

	float getMeanD() {
		if (this == nullptr) {

			return 0;
		}
		else {
			return this->sMajorAxis;
		}
	}

	std::string getName() {
		if (this == nullptr) {
			
			return "";
		}
		else {
			return this->name;
		}
	}

	int getType() {
		if (this == nullptr) {
			return -1;
		}
		else {
			return this->type;
		}
	}
};

//############## FUNCTION DEFS ###########

sf::Vector2f paraPos(float h, float k, float t, float a, float b) {
	sf::Vector2f ans(0,0);

	//Remeber, t heres is a value in radians that correspods to an angle on the unit circle
	ans.x =h + (a * cos(t));
	ans.y =k + (b * sin(t));

	return ans;
}

void togglePause(bool* p) {
	//Toggle Pause Boolean
	*p = !*p;
}

float getDist(sf::Vector2f a, sf::Vector2f b) {
	//Gets distance between two points
	return sqrt(pow(abs(a.x - b.x),2) + pow(abs(a.y - b.y),2));
}

sf::Vector2f getMidP(sf::Vector2f a, sf::Vector2f b) {
	//Gets the midpoint between two points
	sf::Vector2f temp;

		temp.x = (a.x + b.x) / 2.f;
		temp.y = (a.y + b.y) / 2.f;

	return temp;

}

float triArea(sf::Vector2f a, sf::Vector2f b, sf::Vector2f sunPos) {
	//This function takes three points(a triangle) and calculates the
	//area between them

	float base, aToSun, bToSun = 0.f;



	base = getDist(a,b); // base
	aToSun = getDist(a, sunPos);
	bToSun = getDist(b,sunPos); 

	float s = (base + aToSun + bToSun) / 2;

	float area = sqrt(s*(s-aToSun)*(s-bToSun)*(s-base));

	return area;

	//(Base * Height) /2

}

void initGui(tgui::Gui* targ, bool* p) {
	
	//Top Bar
	tgui::Panel::Ptr topBar = tgui::Panel::create();
	sf::Texture tPane;
	tPane.loadFromFile("topBar.png");
	topBar->setSize(1200, 100);
	topBar->getRenderer()->setTextureBackground(tPane);

	targ->add(topBar);

	auto tbL = tgui::Label::create();

	tbL->setText("Keplers 3 Laws:\n1. A planet orbits in an ellipse with the Sun as a focus\n2. The velocity of a planet will change such that a line from it to its sun will sweep out equal areas in equal time\n3. A planets period squared is equal to the cube of its distance from the sun. ");
	tbL->setPosition(100, 30);
	tbL->setTextSize(12);
	targ->add(tbL);

		
		//Play Pause Button
	tgui::Button::Ptr playButton = tgui::Button::create();
	sf::Texture pButton;
	pButton.loadFromFile("playButton.png");
	playButton->getRenderer()->setTexture(pButton);
	playButton->setSize(52,58);
	playButton->setPosition(25,30);
	//playButton->onClick(togglePause(p));


	targ->add(playButton);
	

	//Right Bar
	tgui::Panel::Ptr rightBar = tgui::Panel::create();
	sf::Texture rPane;
	rPane.loadFromFile("rightBar.png");

	rightBar->setSize(200, 650);
	rightBar->setPosition(1000, 100);
	rightBar->getRenderer()->setTextureBackground(rPane);

	targ->add(rightBar);

	//Bottom Bar
	std::vector<tgui::Panel::Ptr> bLst;
	sf::Texture bPane;

	bPane.loadFromFile("bBarR.png");

	tgui::Panel::Ptr botBar0 = tgui::Panel::create();
	bLst.push_back(botBar0);
	tgui::Panel::Ptr botBar1 = tgui::Panel::create();
	bLst.push_back(botBar1);
	tgui::Panel::Ptr botBar2 = tgui::Panel::create();
	bLst.push_back(botBar2);
	tgui::Panel::Ptr botBar3 = tgui::Panel::create();
	bLst.push_back(botBar3);
	tgui::Panel::Ptr botBar4 = tgui::Panel::create();
	bLst.push_back(botBar4);
	tgui::Panel::Ptr botBar5 = tgui::Panel::create();
	bLst.push_back(botBar5);

	for (int i = 0; i < bLst.size(); i++) {
		bLst[i]->setSize(200, 50);
		bLst[i]->setPosition(200 * i, 750);
		bLst[i]->getRenderer()->setTextureBackground(bPane);
		targ->add(bLst[i]);
	}
}


//################ MAIN ##################

int main() {

#pragma region Vars

	//Vars
	sf::RenderWindow wind(sf::VideoMode(1200,800), "SolSim");
	sf::Image icon;
	sf::Clock dt;
	sf::Clock pointTimer;
	sf::Time dur;
	sf::Time deltaTime;
	sf::Event staticEar;
	bool isPaused = false;
	std::vector<sf::Drawable*> drawLst;
	std::vector<Nbody*> nBodyLst;
	Nbody* selected = nullptr;
	int selThresh = 30;

	//UI and Movement
	sf::View bodyVeiw(sf::FloatRect(0,0,1200,800));
	sf::Vector2f offset(0.f,0.f);

	int panSpeed = 800;
	int zoomLvl = 0;

	tgui::Gui mainGui(wind);

#pragma endregion
	
	//Testing

	sf::Vector2f a(1200,800);

	//selbaward::Starfield test(a, 200, sf::Color::Blue);

	int donovan = 0;

	//Setup
		//Window
	icon.loadFromFile("solicon.png");
	wind.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
	
#pragma region UI

		//UI
	initGui(&mainGui, &isPaused);

	auto zoomLabel = tgui::Label::create();

	zoomLabel->setText("Zoom: 0");
	zoomLabel->setPosition(1010, 770);
	zoomLabel->setTextSize(12);
	mainGui.add(zoomLabel);

	auto selNameL = tgui::Label::create();
	selNameL->setText("Name: ");
	selNameL->setPosition(1100, 130);
	selNameL->setTextSize(12);
	mainGui.add(selNameL);

	auto selName = tgui::Label::create();
	selName->setText("");
	selName->setPosition(1100, 145);
	selName->setTextSize(12);
	mainGui.add(selName);

	auto selV = tgui::Label::create();
	selV->setText("Velocity: ");
	selV->setPosition(1022, 200);
	selV->setTextSize(12);
	mainGui.add(selV);

	auto selDist = tgui::Label::create();
	selDist->setText("Distance: ");
	selDist->setPosition(1022, 215);
	selDist->setTextSize(12);
	mainGui.add(selDist);

	auto selODist = tgui::Label::create();
	selODist->setText("Mean Distance: ");
	selODist->setPosition(1022, 230);
	selODist->setTextSize(12);
	mainGui.add(selODist);

	auto selOrbits = tgui::Label::create();
	selOrbits->setText("Orbits Completed: ");
	selOrbits->setPosition(1022, 245);
	selOrbits->setTextSize(12);
	mainGui.add(selOrbits);

	sf::Texture p, j, s, e;

	p.loadFromFile("earthG.png");
	j.loadFromFile("jovianG.png");
	s.loadFromFile("sunG.png");
	e.loadFromFile("noneG.png");

	auto dispG = tgui::Picture::create();
	dispG->getRenderer()->setTexture(e);
	dispG->setPosition(1035, 130);
	dispG->setSize(45,45);

	mainGui.add(dispG);




#pragma endregion

#pragma region Solar Bodies

		//Solar Bodies
	Nbody sun("Euphrates", 2, 60.f, 0, 0, 0,2e30, wind.getSize().x / 2.0, wind.getSize().y / 2.0);
	Nbody planet("Euphrates I", 0, 20.f, 349.2, 29.78, 0.5, 6e2, 0, 0);
	Nbody jovian("Euphrates III", 1, 30.f, 778, 13.06, 0.04, 1.8e27, 0, 0);
	Nbody mars("Euphrates II", 0, 16.f, 523.5, 24.07,0.093, 5e2, 0, 0);
	Nbody moon("euprates IA", 0, 3.f, 38.25f, 3.825f, 0, 7e22, 0, 0);

	planet.setParent(&sun);
	//moon.setParent(&planet);
	jovian.setParent(&sun);
	mars.setParent(&sun);

	nBodyLst.push_back(&sun);
	nBodyLst.push_back(&planet);
	nBodyLst.push_back(&jovian);
	nBodyLst.push_back(&mars);
	//nBodyLst.push_back(&moon);

	for (int i = 0; i < nBodyLst.size(); i++) {
		drawLst.push_back(nBodyLst[i]->getGraphic());
	}
#pragma endregion
	
	//Window Loop
	while (wind.isOpen()) {

#pragma region Static Events
		//Static Events
		while (wind.pollEvent(staticEar)) {

			//Gui
			mainGui.handleEvent(staticEar);

	#pragma region Select Planet

				//Body Select
			Nbody* temp = nullptr;

			if (staticEar.type == sf::Event::MouseButtonReleased && staticEar.mouseButton.button == sf::Mouse::Left) {
				for (int i = 0; i < nBodyLst.size(); i++) {

					int t1 = abs(sf::Mouse::getPosition(wind).x - nBodyLst[i]->getCenter().x + offset.x);
					int t2 = abs(sf::Mouse::getPosition(wind).y - nBodyLst[i]->getCenter().y + offset.y);

					if (t1 < selThresh && t2 < selThresh) {
						temp = nBodyLst[i];
					}
				}

				selected = temp;
				temp = nullptr;

				

				
				std::cout << sf::Mouse::getPosition(wind).x<< ", "<< sf::Mouse::getPosition(wind).y << endl;
			}

	#pragma endregion

			//Regular
			if (staticEar.type == sf::Event::Closed) {
				wind.close();
			}

			if (staticEar.type == sf::Event::KeyReleased && staticEar.key.code == sf::Keyboard::Space) {
				if (isPaused) {
					isPaused = false;
					std::cout << "Resuming" << endl;
				}
				else
				{
					isPaused = true;
					std::cout << "Pausing" << endl;
				}
				
			}

			//Scroll Zoom
			if (staticEar.type == sf::Event::MouseWheelScrolled) {
				//ADD ACCOUNTING FOR COORD OFFSET!!!
				if (staticEar.mouseWheelScroll.delta == 1 && zoomLvl <= 5) {
					//Zoom in
					bodyVeiw.zoom(0.8);
					panSpeed *= 0.8;
					//offset.x /= 0.8;
					//offset.y /= 0.8;
					zoomLvl++;
				}
				else if(staticEar.mouseWheelScroll.delta == -1 && zoomLvl >= -5) {
					//Zoom Out 
					bodyVeiw.zoom(1.25);
					panSpeed *= 1.25;
					//offset.x *= 1.25;
					//offset.y *= 1.25;
					zoomLvl--;
				}

				zoomLabel->setText("Zoom: " + std::to_string(zoomLvl));
			}
			//
		}

#pragma endregion

#pragma region Camera panning

		//Camera Controls

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
			//Move 0, -
			bodyVeiw.move(0, -1 * panSpeed * deltaTime.asSeconds());
			offset.y -= panSpeed * deltaTime.asSeconds();
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
			//Move 0, +
			bodyVeiw.move(0, panSpeed * deltaTime.asSeconds());
			offset.y += panSpeed * deltaTime.asSeconds();
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
			//Move -, 0
			bodyVeiw.move(-1 * panSpeed * deltaTime.asSeconds(), 0);
			offset.x -= panSpeed * deltaTime.asSeconds();
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
			//Move +, 0
			bodyVeiw.move(panSpeed * deltaTime.asSeconds(), 0);
			offset.x += panSpeed * deltaTime.asSeconds();
		}

#pragma endregion

		planet.orbit(isPaused);
		//moon.orbit(isPaused);
		jovian.orbit(isPaused);
		mars.orbit(isPaused);

		//std::std::cout << "Vel: " << planet.getVel() << std::endl;
		//std::std::cout << "Distance: " << moon.getParDist() << std::endl;

		//Update Views
		wind.setView(bodyVeiw);

		//Update dt
		deltaTime = dt.restart();

		//Update info panel
		if (selected == nullptr) {
			selName->setText("");
			selV->setText("Velocity: ");
			selDist->setText("Distance: ");
			selODist->setText("Mean Distance: ");
			selOrbits->setText("Completed Orbits: ");
			dispG->getRenderer()->setTexture(e);
		}
		else {
			selName->setText(selected->getName());
			selV->setText("Velocity: " + std::to_string((int)selected->getVel()));
			selDist->setText("Distance: " + std::to_string((int)selected->getParDist()));
			selODist->setText("Mean Distance: " + std::to_string((int)selected->getMeanD()));
			selOrbits->setText("Completed Orbits: " + std::to_string(selected->getNumOrbits()));
			if (selected->getType() == 0) {
				dispG->getRenderer()->setTexture(p);
			}
			else if (selected->getType() == 1) {
				dispG->getRenderer()->setTexture(j);
			}
			else {
				dispG->getRenderer()->setTexture(s);
			}
		}

		//Clear, Draw, and Display
		wind.clear();
		

		for (int i = 0; i < drawLst.size(); i++) {
			wind.draw(*drawLst[i]);
		}

		if (selected == nullptr || selected->getParent() == nullptr) {
			for(int i = 0; i < nBodyLst.size(); i++) {
				nBodyLst[i]->draw(&wind);
			}
		}
		else {
			selected->draw(&wind);
		}

		mainGui.draw();

		wind.display();

	}


	return 0;
}