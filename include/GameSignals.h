
#ifndef __GAME_CONTROLLER_H__
#define __GAME_CONTROLLER_H__

#include <boost/signals2.hpp>
#include <boost/utility/singleton.hpp>

// GameSignals class is a singleton. There is only one class that manages signals
class GameSignals : public boost::singleton<GameSignals>
{
	public:
		//This constructor is some Boost magic to avoid accidental construction of GameSignals object
		GameSignals(const boost::restricted) {}


		//Define some signal / event types
		typedef boost::signals2::signal< void (float) > collide_bomb_sig_t;


		// Connect a callback to the bomb collision signal
		void onBombExploded( const collide_bomb_sig_t::slot_type& slot );
		// Generate a bomb collision signal
		void doBombExplode(float damage);

	private:
		//Create signal instances
		collide_bomb_sig_t _bomb_collision;
};

//Connect callbacks to the bomb collision signal
void GameSignals::onBombExploded( const collide_bomb_sig_t::slot_type& slot )
{
	_bomb_collision.connect(slot);
}

void GameSignals::doBombExplode(float damage)
{
	//Dispatch the bomb collision signal with some damage.
	_bomb_collision( damage );
}



#endif
