/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
*/
#ifndef ZYPP_NG_BASE_STATEMACHINE_INCLUDED_H
#define ZYPP_NG_BASE_STATEMACHINE_INCLUDED_H

#include <zypp/zyppng/base/signals.h>
#include <zypp/zyppng/base/Base>

#include <variant>
#include <tuple>
#include <functional>
#include <memory>

namespace zyppng {

  namespace detail {

    template <typename T>
    using EventSource = SignalProxy<void()> (T::*)();

    /**
     * \internal
     * Internal helper type to wrap the user implemented state types.
     * It's mostly used to have a container for the Transitions that belong to the State.
     */
    template < class State, class Transitions >
    struct StateWithTransitions {
      using StateType = State;
      Transitions _transitions;

      template< typename StateMachine >
      StateWithTransitions ( StateMachine &sm ) : _ptr ( std::make_shared<State>( sm )) { }
      StateWithTransitions ( std::shared_ptr<State> &&s ) : _ptr ( std::move(s) ) {}

      // move construction is ok
      StateWithTransitions ( StateWithTransitions &&other ) = default;
      StateWithTransitions &operator= ( StateWithTransitions &&other ) = default;

      // no copy construction
      StateWithTransitions ( const StateWithTransitions &other ) = delete;
      StateWithTransitions &operator= ( const StateWithTransitions &other ) = delete;

      static constexpr auto stateId = State::stateId;
      static constexpr bool isFinal = State::isFinal;

      void enter( ) {
        return _ptr->enter( );
      }

      void exit( ) {
        return _ptr->exit( );
      }

      operator State& () {
        return wrappedState();
      }

      State &wrappedState () {
        return *_ptr;
      }

      const State &wrappedState () const {
        return *_ptr;
      }

    private:
      // we need to use a std::shared_ptr here so we can correctly reference the object during signal emission
      std::shared_ptr<State> _ptr;
    };


    /**
     * this adds the type \a NewType to the collection if the condition is true
     */
    template < template<typename...> typename Templ , typename NewType, typename TupleType, bool condition >
    struct add_type_to_collection;

    template < template<typename...> typename Templ, typename NewType, typename ...Types >
    struct add_type_to_collection< Templ, NewType, Templ<Types...>, true > {
      using Type = Templ<Types..., NewType>;
    };

    template < template<typename...> typename Templ, typename NewType, typename ...Types >
    struct add_type_to_collection< Templ, NewType, Templ<Types...>, false > {
      using Type = Templ<Types...>;
    };

    /**
     * Constexpr function that evaluates to true if a variant type \a Variant already contains the type \a Type.
     * The \a Compare argument can be used to change how equality of a type is calculated
     */
    template < typename Variant, typename Type, template<typename, typename> typename Compare = std::is_same, size_t I = 0 >
    constexpr bool VariantHasType () {
      // cancel the evaluation if we entered the last type in the variant
      if constexpr ( I >= std::variant_size_v<Variant> ) {
        return false;
      } else {
        // if the current type in the variant is the same as the one we are looking for evaluate to true
        if ( Compare< std::variant_alternative_t< I, Variant>, Type >::value )
          return true;

        // otherwise call the next iteration with I+1
        return VariantHasType<Variant, Type, Compare, I+1>();
      }
    }

    /**
     * collect all transitions that have the same SourceState as the first type argument
     */
    template< class State, class TupleSoFar, class Head, class ...Transitions >
    struct collect_transitions_helper {
      using NewTuple = typename add_type_to_collection< std::tuple, Head, TupleSoFar, std::is_same_v< State, typename Head::SourceType> >::Type;
      using Type = typename collect_transitions_helper<State, NewTuple, Transitions...>::Type;
    };

    template< class State, class TupleSoFar, class Head >
    struct collect_transitions_helper<State, TupleSoFar, Head > {
      using Type = typename add_type_to_collection< std::tuple, Head, TupleSoFar, std::is_same_v< State, typename Head::SourceType> >::Type;
    };

    template< class State, class ...Transitions >
    struct collect_transitions{
      using Type = typename collect_transitions_helper< State, std::tuple<>, Transitions... >::Type;
    };

    /**
     * Iterates over the list of Transitions and collects them all in a std::variant<State1, State2, ...> type
     */
    template <typename VariantSoFar, typename Head, typename ...Transitions>
    struct make_state_set_helper {
      using WithSource = typename add_type_to_collection< std::variant, typename Head::SourceType, VariantSoFar, !VariantHasType<VariantSoFar, typename Head::SourceType>() >::Type;
      using WithTarget = typename add_type_to_collection< std::variant, typename Head::TargetType, WithSource, !VariantHasType<WithSource, typename Head::TargetType>() >::Type;
      using Type = typename make_state_set_helper<WithTarget, Transitions...>::Type;
    };

    template <typename VariantSoFar, typename Head>
    struct make_state_set_helper< VariantSoFar, Head > {
      using WithSource = typename add_type_to_collection< std::variant, typename Head::SourceType, VariantSoFar, !VariantHasType<VariantSoFar, typename Head::SourceType>() >::Type;
      using Type = typename add_type_to_collection< std::variant, typename Head::TargetType, WithSource, !VariantHasType<WithSource, typename Head::TargetType>() >::Type;
    };

    template <typename Head, typename ...Transitions>
    struct make_state_set {
      using InitialVariant = std::variant<typename Head::SourceType>;
      using VariantSoFar = typename add_type_to_collection< std::variant, typename Head::TargetType, InitialVariant , !VariantHasType<InitialVariant, typename Head::TargetType>() >::Type;
      using Type = typename make_state_set_helper< VariantSoFar, Transitions...>::Type;
    };


    /**
     * Evaluates to true if type \a A and type \a B wrap the same State type
     */
    template <typename A, typename B>
    struct is_same_state : public std::is_same< typename A::StateType, typename B::StateType> {};


    /**
     * Turns a State type into its StateWithTransitions counterpart
     */
    template <typename State, typename ...Transitions>
    struct make_statewithtransition {
      using Type = StateWithTransitions<State, typename collect_transitions<State, Transitions...>::Type>;
    };

    /**
     * Iterates over each State in the \a StateVariant argument, collects the corresponding
     * Transitions and combines the results in a std::variant< StateWithTransitions<...>,... > type.
     */
    template <typename VariantSoFar, typename StateVariant, typename ...Transitions>
    struct make_statewithtransition_set_helper;

    template <typename VariantSoFar, typename HeadState, typename ...State, typename ...Transitions>
    struct make_statewithtransition_set_helper< VariantSoFar, std::variant<HeadState, State...>, Transitions... > {
      using FullStateType = typename make_statewithtransition<HeadState, Transitions...>::Type;
      using NewVariant = typename add_type_to_collection< std::variant, FullStateType, VariantSoFar, !VariantHasType<VariantSoFar, FullStateType/*, is_same_state */>()>::Type;
      using Type = typename make_statewithtransition_set_helper< NewVariant, std::variant<State...>, Transitions...>::Type;
    };

    template <typename VariantSoFar, typename HeadState, typename ...Transitions >
    struct make_statewithtransition_set_helper< VariantSoFar, std::variant<HeadState>, Transitions... > {
      using FullStateType = typename make_statewithtransition<HeadState, Transitions...>::Type;
      using Type = typename add_type_to_collection< std::variant, FullStateType, VariantSoFar, !VariantHasType<VariantSoFar, FullStateType /*, is_same_state */>()>::Type;
    };

    template <typename NoState, typename StateVariant, typename ...Transitions>
    struct make_statewithtransition_set;

    template <typename NoState, typename HeadState, typename ...States, typename ...Transitions>
    struct make_statewithtransition_set< NoState, std::variant<HeadState, States...>, Transitions...>{
      using FirstState = typename make_statewithtransition< HeadState, Transitions...>::Type;
      using Type = typename make_statewithtransition_set_helper< std::variant<NoState, FirstState>, std::variant<States...>, Transitions...>::Type;
    };
  }

  constexpr bool DefaultStateCondition(true);
  constexpr std::nullptr_t DefaultStateTransition(nullptr);

  /*!
   * Defines a transition between \a Source and \a Target states.
   * The EventSource \a ev triggers the transition from Source to Target if the condition \a Cond
   * evaluates to true. The operation \a Op is called between exiting the old and entering the new state.
   * It can be used to transfer informations from the old into the new state.
   *
   * \tparam Source defines the type of the Source state
   * \tparam ev takes a member function pointer returning the event trigger signal that is used to trigger the transition to \a Target
   * \tparam Target defines the type of the Target state
   * \tparam Cond Defines the transition condition, can be used if the same event could trigger different transitions
   *         based on a condition, this can also be set to a simple boolean true or false
   * \tparam Op defines the transition operation from Source to Target states,
   *         this is either a function with the signature:   std::unique_ptr<Target> ( Statemachine &, Source & )
   *         or it can be a member function pointer of Source with the signature:  std::unique_ptr<Target> ( Source::* ) ( )
   *
   * \note   While it would be possible to implement the statemachine to operate only on non pointer types for the states ,
   *         I chose to use std::unique_ptr<State> instead to make the handling of States with signals less error prone. Because even move
   *         assigning a signal that has connected lambda slots which have captured the this pointer will break the code. While it could
   *         be worked around to connect and disconnect signals in the enter() and exit() functions not doing so would crash the code. Leaving that
   *         note here in case we want to change that behaviour in the future.
   */
  template <
    typename Source,
    detail::EventSource<Source> ev ,
    typename Target,
    auto Cond = DefaultStateCondition,
    auto Op   = DefaultStateTransition >
  struct Transition {

    using SourceType = Source;
    using TargetType = Target;


    template< typename Statemachine >
    std::shared_ptr<Target> operator() ( Statemachine &sm, Source &oldState ) {
      using OpType = std::decay_t<decltype ( Op )>;
      // check if we have a member function pointer
      if constexpr (  std::is_member_function_pointer_v<OpType> ) {
        return std::invoke( Op, &oldState );
      } else if constexpr ( std::is_null_pointer_v<OpType> ) {
        return std::make_shared<Target>(sm);
      } else {
        return std::invoke( Op, sm, oldState );
      }
    }

    bool checkCondition ( Source &currentState ) {
      using CondType = std::decay_t<decltype ( Cond )>;
      if constexpr ( std::is_same_v<bool, CondType> ) {
        return Cond;
      } else if constexpr ( std::is_member_function_pointer_v<CondType> ) {
        return std::invoke( Cond, &currentState );
      } else {
        return std::invoke( Cond, currentState );
      }
    }

    SignalProxy< void() > eventSource ( Source *st ) {
      return std::invoke( ev, st );
    }

    auto eventAccessor () const {
      return ev;
    }

  };

  /*!
   * \brief This defines the actual StateMachine.
   * \tparam Derived is the Statemachine subclass type, this is used to pass a reference to the actual implementation into the State functions.
   * \tparam StateId should be a enum with a ID for each state the SM can be in.
   * \tparam Transitions variadic template argument taking a list of all \ref Transition types the statemachine should support.
   *         The First Source State in the Transitions List is always the initial state.
   *
   * Implementation of a simple signal based statemachine, each state is a user defined state type that has to implement a specific API in order to be compatible.
   * No inheritance is required to use the statemachine and everything will be resolved at compile time.
   *
   * This is how a basic statemachine implementation would look like
   *
   * \code
   *
   * // Each state should have a representation in the enum
   * enum States {
   *  StateA,
   *  StateB,
   *  ...
   * }
   *
   * class MyStateMachine;
   * class State;
   * class StateB;
   * class StateC;
   * class StateD;
   *
   * // states are just simple types with a few required functions, there is a helper class
   * // SimpleState<> to help with the implementation
   * class StateA {
   *
   *  // the ID of the state
   *  static constexpr auto stateId = States::StateA;
   *
   *  // is this a final state?
   *  static constexpr bool isFinal = false;
   *
   *  // constructor taking the parent statemachine type
   *  StateA ( MyStateMachine &parent ) {}
   *
   *  // function called when the state is first entered
   *  void enter() {}
   *
   *  // function called when the state is exited
   *  void exit () {}
   *
   *  // a state that is not final needs event sources
   *  SignalProxy<void()> sigTransition ();
   *
   *  // a state might even define a transition function to the target states
   *  std::unique_ptr<StateB> transitionToB ();
   *
   *  // also condition functions can be defined in a state:
   *  bool transitionToStateBCondition() const;
   * }
   *
   * // transition functions can also be defined as free functions or static member functions with the signature:
   * std::unique_ptr<StateC> transitionStateBToStateC ( MyStateMachine &sm, StateB &oldState );
   *
   * // condition functions can also be defined as free functions or static member functions with the signature:
   * bool transitionStateAToStateDCondition( State &currentState );
   *
   *
   * // implementing the statemachine itself, using a template helper for better readability:
   * template <typename T>
   * using SmBase     = zyppng::Statemachine<T, States,
   *                                         zyppng::Transition< StateA, &StateA::sigTransition, StateB, &StateA::transitionToStateBCondition, &StateA::transitionToB,
   *                                         zyppng::Transition< StateB, &StateB::sigTransition, StateC, zyppng::DefaultStateCondition, &transitionStateBToStateC >,
   *                                         zyppng::Transition< StateB, &StateB::sigTransition, StateD >>;
   *
   * // by using CRTP we can have a reference to the concrete Statemachine type in the States, allowing us to be much
   * // more flexible with the implementation, since all states can now access API exported by the MyStateMachine type.
   * // since the Statemachine uses signals, make sure to derive it also from the \ref zyppng::Base type.
   * class MyStateMachine : public SmBase<MyStateMachine>, public Base { };
   *
   * \endcode
   *
   * In order to advance the statemachine, each state will have at least one signal ( event ) that tells the statemachine to transition to the next state, optionally
   * a condition can be used to block the transition in certain cases. When transitioning from one state to the other a transition operation will be called that
   * creates the instance of the target state and do other initializations or can move data from the old to the new state. The default version of the operation just
   * returns a new instance of the target state.
   * If a signal or event is used multiple times to trigger transitions, only the first transition whose condition evaluates to true will be triggered. All other transitions
   * with the same trigger signal will not be even evaluated.
   *
   * After instantiating the statemachine will be in a internal intial state, in order to move to the first user defined state \ref Statemachine::start must be called.
   */
  template < typename Derived, typename StateId, typename ...Transitions >
  class Statemachine {

    struct _InitialState{};

  public:

    using AllStates = typename detail::make_state_set< Transitions... >::Type;
    using StateSetHelper = typename detail::make_statewithtransition_set< _InitialState, AllStates, Transitions... >;
    using FState = typename StateSetHelper::FirstState;
    using StateSet = typename StateSetHelper::Type;

    using StatemachineType = Statemachine< Derived, StateId, Transitions...>;

    public:
      Statemachine () { }
      virtual ~Statemachine() {}

      /*!
       * Advances the state machine into the initial state.
       */
      void start () {
        if ( _state.index() == 0 || _isInFinalState ) {
          _previousState.reset();
          _isInFinalState = false;
          enterState( FState( static_cast<Derived &>(*this) ) );
        }
      }

      template <typename Func>
      auto visitState ( Func && f ) {
        return std::visit( [ func = std::forward<Func>(f) ] ( auto &s ) {
          using T = std::decay_t<decltype (s)>;
          if constexpr ( std::is_same_v< T, _InitialState > ) {
            throw std::exception();
          } else {
            return ( func( s.wrappedState() ) );
          }
        }, _state );
      }

      /*!
       * Returns the current stateId of the state the SM is currently in.
       * If called before start() was called the std::optional will be empty
       */
      std::optional<StateId> currentState () const {
        return std::visit( []( const auto &s ) -> std::optional<StateId> {
          using T = std::decay_t<decltype (s)>;
          if constexpr ( std::is_same_v< T, _InitialState > ) {
            return {};
          } else {
            return T::stateId;
          }
        }, _state );
      }

      /*!
       * Returns the ID of the previous state, or a invalid optional if there
       * was no previous state.
       */
      std::optional<StateId> previousState () const {
        return _previousState;
      }

      /*!
       * Returns a reference to the current state object, will throw a exception
       * if the passed state type does not match the current states type.
       */
      template<typename T>
      T& state () {
        using WrappedEventType = typename detail::make_statewithtransition< std::decay_t<T>, Transitions...>::Type;
        return std::get<WrappedEventType>( _state ).wrappedState();
      }

      /*!
       * Returns a reference to the current state object, will throw a exception
       * if the passed state type does not match the current states type.
       */
      template<typename T>
      const T& state () const {
        using WrappedEventType = typename detail::make_statewithtransition< std::decay_t<T>, Transitions...>::Type;
        return std::get<WrappedEventType>( _state ).wrappedState();
      }

      /*!
       * Forces the statemachine to enter a specific state, a transition operation will not
       * be executed, but the exit() function of the current state will be called.
       */
      template <typename NewState >
      void forceState ( std::unique_ptr<NewState> &&nS ) {
        using WrappedSType = typename detail::make_statewithtransition< std::decay_t<NewState>, Transitions...>::Type;
        std::visit( [this, &nS]( auto &currState ) {
          using T = std::decay_t<decltype (currState)>;
          if constexpr ( std::is_same_v< T, _InitialState > ) {
            enterState ( WrappedSType( std::move(nS) ) );
          } else {
            enterState ( currState, WrappedSType( std::move(nS) ) );
          }
        }, _state );
      }

      /*!
       * Emitted when the statemachine enters a type that has final set to true.
       */
      SignalProxy<void()> sigFinished () {
        return _sigFinished;
      }

      /*!
       * Emitted everytime the statemachine advanced to a new state, carrying the
       * new state's ID.
       * \note this signal is emitted before State::enter() is executed, the State object
       *       however is already created and can be accessed via \ref Statemachine::state()
       */
      SignalProxy<void ( StateId )> sigStateChanged () {
        return _sigStateChanged;
      }

    protected:

      template <typename OldState, typename NewState>
      void enterState ( OldState &os, NewState &&nS ) {
        // disconnect all signals from the current state
        clearConnections();
        std::forward<OldState>(os).exit();
        _previousState = OldState::stateId;
        enterState( std::forward<NewState>(nS) );
      }

      template <typename NewState>
      void enterState ( NewState &&nS ) {

        if constexpr ( !NewState::isFinal ) {
          connectAllTransitions<0>( nS, nS._transitions );
        }

        _state = std::forward<NewState>(nS);

        // handle final state things
        if constexpr ( NewState::isFinal ) {
          _isInFinalState = true;

          // let the outside world know whats going on
          _sigStateChanged.emit( NewState::stateId  );

          _sigFinished.emit();
        } else {
          // if we enter a non final state we only emit one signal
          _sigStateChanged.emit( NewState::stateId  );
        }

        // call enter on the state as the last thing to do, it might emit a transition event right away
        std::get< std::decay_t<NewState> >( _state ).enter();
      }

      template <typename State, typename Transition>
      auto makeEventCallback ( Transition &transition ) {
        using WrappedEventType = typename detail::make_statewithtransition< typename Transition::TargetType, Transitions...>::Type;
        return [ mytrans = &transition, this]() mutable {
          if ( mytrans->checkCondition( std::get< State >(_state) ) ) {
            auto &st = std::get< std::decay_t<State> >(_state);
            enterState( st , WrappedEventType( (*mytrans)( static_cast<Derived &>(*this), st ) ) );
          }
        };
      }

      template< std::size_t I = 0, typename State, typename ...StateTrans>
      void connectAllTransitions( State &&nS, std::tuple<StateTrans...> &transitions ) {
        if constexpr (I >= sizeof...(StateTrans)) {
          return;
        } else {
          auto &transition = std::get<I>( transitions );
          //_currentStateConnections.push_back( transition.eventSource ( &std::forward<State>(nS).wrappedState() ).connect( makeEventCallback< std::decay_t<State> >(transition)) );
          _currentStateConnections.push_back( std::forward<State>(nS).wrappedState().Base::connectFunc( transition.eventAccessor(), makeEventCallback< std::decay_t<State> >(transition), *static_cast<Derived*>(this) ) );
          connectAllTransitions<I+1>( std::forward<State>(nS), transitions );
        }
      }

      void clearConnections () {
        for ( auto &c : _currentStateConnections )
          c.disconnect();
        _currentStateConnections.clear();
      }

    private:
      bool _isInFinalState = false;
      Signal <void ( StateId )> _sigStateChanged;
      Signal <void ()> _sigFinished;
      StateSet _state = _InitialState();
      std::optional<StateId> _previousState;
      std::vector<sigc::connection> _currentStateConnections;
  };

  /*!
   * Implements the most basic parts of a State,
   * the functions enter() and exit() still need to be implemented in the base class
   */
  template < typename StatemachineType, bool isFin >
  class BasicState : public Base {
  public:

    static constexpr bool isFinal = isFin;

    BasicState( StatemachineType &sm ) : _sm( sm ){}
    virtual ~BasicState() {}

    BasicState( BasicState && ) = default;
    BasicState &operator= ( BasicState && ) = default;

    StatemachineType &stateMachine () {
      return _sm;
    }

    const StatemachineType &stateMachine () const {
      return _sm;
    }

  private:
    StatemachineType &_sm;

  };

  /*!
   * Helper type that also includes the State ID, in more complex state machines
   * it might be desireable to have the State ID in the final class implementation
   */
  template < typename StatemachineType, auto sId, bool isFin >
  class SimpleState : public BasicState<StatemachineType, isFin> {
    public:
      static constexpr auto stateId = sId;
      using BasicState<StatemachineType, isFin>::BasicState;
  };

}

#endif // ZYPP_NG_BASE_STATEMACHINE_INCLUDED_H
