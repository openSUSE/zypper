#ifndef OUTPUT_H
#define OUTPUT_H

#include <zypp-core/zyppng/base/Base>
#include <ncpp/NotCurses.hh>
#include <ncpp/Plane.hh>
#include <ncpp/MultiSelector.hh>
#include <ncpp/Reader.hh>
#include <memory>


static void zypp_release_ncplane ( struct ncplane *ptr ) { if ( ptr ) ncplane_destroy(ptr); }
static void zypp_release_progbar ( struct ncprogbar *ptr ) { if ( ptr ) ncprogbar_destroy(ptr); }

class OutputView : public zyppng::Base
{
  public:
    ~OutputView() {
    }
    static std::shared_ptr<OutputView> create( ) {

      auto ptr = std::shared_ptr<OutputView>( new OutputView() );
      ptr->_nc = std::make_unique<ncpp::NotCurses>();
      if ( !ptr->_nc )
        return nullptr;

      ptr->_stdplane = std::unique_ptr<ncpp::Plane>( ptr->_nc->get_stdplane() );
      if ( !ptr->_stdplane )
        return nullptr;

      int rows = 0;
      int cols = 0;
      ptr->_stdplane->get_dim( rows, cols );

      int top_rows = ( rows - 2 );
      if ( top_rows <= 0 ){
        return nullptr;
      }

      int bottom_rows = rows - top_rows;
      if ( bottom_rows <= 0 ) {
        return nullptr;
      }

      // the outer plane this is just used to draw a nice border
      ptr->_topOuterPlane = std::make_unique<ncpp::Plane>( *ptr->_stdplane, top_rows, cols, 0, 0, nullptr );
      ptr->_topOuterPlane->rounded_box_sized( 0, 0, top_rows, cols, 0 );

      // the inner plane, here we put the text
      int r,c;
      ptr->_topOuterPlane->get_dim( r, c );
      ptr->_topPlaneLeft = std::make_unique<ncpp::Plane>( *ptr->_topOuterPlane, r-2 , (c-1) / 2, 1, 1 );
      ptr->_topPlaneLeft->set_scrolling( true );

      ptr->_topPlaneRight = std::make_unique<ncpp::Plane>( *ptr->_topOuterPlane, r-2 , c - ptr->_topPlaneLeft->get_dim_x() - 1 , 1, ptr->_topPlaneLeft->get_dim_x() + 1  );
      ptr->_topPlaneRight->set_scrolling( true );

      // the plane where we show the progressbar text
      ptr->_progBarTextPlane = std::make_unique<ncpp::Plane>( *ptr->_stdplane, 1 , cols, top_rows, 0 );

      // to create the progressbar we need to fall back to the C API due to some bugs in the C++ progbar implementation
      ncplane_options nopts = {
        .y = top_rows+1,
        .x = 0,
        .rows = bottom_rows-1,
        .cols = cols,
        .userptr = nullptr,
        .name = nullptr,
        .resizecb = nullptr,
        .flags = 0,
        .margin_b = 0,
        .margin_r = 0,
      };

      std::unique_ptr<struct ncplane, void(*)(struct ncplane*)> bottomPlane( ncplane_create( *ptr->_stdplane, &nopts ), &zypp_release_ncplane );
      if ( !bottomPlane ) {
        return nullptr;
      }

      struct ncprogbar_options popts = {
        .ulchannel = 0,
        .urchannel = 0,
        .blchannel = 0,
        .brchannel = 0,
        .flags = 0,
      };
      ptr->_progbar = std::unique_ptr<struct ncprogbar, void(*)(struct ncprogbar*)> ( ncprogbar_create( bottomPlane.release(), &popts ), &zypp_release_progbar );
      if ( !ptr->_progbar ) {
        return nullptr;
      }
      return ptr;
    }

    void updateProgress ( const std::string &txt, double prog, bool autoRender = true ) {
      if ( !_progbar )
        return;

      _progBarTextPlane->erase();
      _progBarTextPlane->putstr( 0, ncpp::NCAlign::Center, txt.data() );
      ncprogbar_set_progress( _progbar.get(), prog );
      if ( autoRender )
        renderNow();
    }

    void putMsgTxt ( const std::string &txt, bool doAutoRender = true ) {
      if ( !_topPlaneLeft )
        return;

      ncplane_puttext( *_topPlaneLeft, -1, NCALIGN_LEFT, txt.data(), nullptr );
      if ( doAutoRender )
        renderNow();
    }

    void putMsgErr ( const std::string &txt, bool doAutoRender = true ) {
      if ( !_topPlaneRight )
        return;

      _topPlaneRight->set_fg_rgb( 0xff5349 );
      ncplane_puttext( *_topPlaneRight, -1, NCALIGN_LEFT, txt.data(), nullptr );
      _topPlaneRight->set_fg_default();
      if ( doAutoRender )
        renderNow();
    }

    std::optional<std::string> promptUser ( const std::string &desc, const std::string &label )
    {
      if ( desc.size() )
        putMsgTxt( desc, false  );
      putMsgTxt( label + ": " );

      int curX, curY;
      _topPlaneLeft->get_cursor_yx( curY, curX  );
      ncplane_options nopts = {
        .y = curY,
        .x = curX,
        .rows = 1,
        .cols = _topPlaneLeft->get_dim_x() - curX,
        .userptr = nullptr,
        .name = nullptr,
        .resizecb = nullptr,
        .flags = 0,
        .margin_b = 0,
        .margin_r = 0,
      };

      std::unique_ptr<struct ncplane, void(*)(struct ncplane*)> inputPlane( ncplane_create( *_topPlaneLeft, &nopts ), &zypp_release_ncplane );
      if ( !inputPlane ) {
        return {};
      }

      auto readOpts = ncreader_options {
        .tchannels = 0,
        .tattrword = 0,
        .flags = NCREADER_OPTION_CURSOR | NCREADER_OPTION_NOCMDKEYS
      };
      std::unique_ptr<struct ncreader, void(*)(struct ncreader*)> reader( ncreader_create( inputPlane.release(), &readOpts ), []( struct ncreader *relme){ ncreader_destroy(relme, nullptr);} );
      if ( !reader )
        return {};

      while ( true ) {
        renderNow();
        ncinput ni;
        _nc->get( true, &ni );
        if ( ni.id == NCKEY_ENTER ) {
          break;
        }
        ncreader_offer_input( reader.get(), &ni );
      }

      std::unique_ptr<char, void(*)(void *)> data( ncreader_contents( reader.get() ), free );
      std::string_view cppData( data.get() );
      if ( cppData.empty() )
        return {};

      return std::string(cppData);
    }

    template <typename T>
    std::vector<int> promptMultiSelect( const std::string &title, const std::string &secondary, const std::vector<T> &data ) {

      // use a boost container here, because the stdlib decided to completely break vector<bool> by prematurely optimizing it
      boost::container::vector<bool> selected( data.size(), false );
      std::vector<ncmselector_item> items;
      std::vector<std::pair<std::string, std::string>> stringsStorage;
      items.reserve( data.size() );
      stringsStorage.reserve( data.size() );

      for ( const T& entry : data ) {
        stringsStorage.push_back( std::make_pair( entry.asUserString(), std::string("") ) );
        items.push_back( ncmselector_item{
          .option = stringsStorage.back().first.data(),
          .desc   = stringsStorage.back().second.data(),
          .selected = false
        });
      }
      items.push_back(ncmselector_item{
        .option = nullptr,
        .desc   = nullptr,
        .selected = false
      });

      std::string_view footer("Press Enter to accept or ESC to cancel.");

      auto selOpts = ncmultiselector_options{
        .title          = title.data(),
        .secondary      = secondary.data(),
        .footer         = footer.data(),
        .items          =  items.data(),
        .maxdisplay     = 0,
        .opchannels     = NCCHANNELS_INITIALIZER(0xe0, 0x80, 0x40, 0, 0, 0),
        .descchannels   = NCCHANNELS_INITIALIZER(0x80, 0xe0, 0x40, 0, 0, 0),
        .titlechannels  = NCCHANNELS_INITIALIZER(0x20, 0xff, 0xff, 0, 0, 0x20),
        .footchannels   = NCCHANNELS_INITIALIZER(0xe0, 0, 0x40, 0x20, 0x20, 0),
        .boxchannels    = NCCHANNELS_INITIALIZER(0x20, 0xe0, 0xe0, 0x20, 0, 0),
        .flags          = 0
      };

      auto tmpPlane = ncpp::Plane( *_stdplane, _stdplane->get_dim_y()-1, _stdplane->get_dim_x()-1, 1, 1 );
      ncpp::MultiSelector selector( tmpPlane, &selOpts);

      while ( true ) {
        renderNow();
        ncinput ni;
        _nc->get( true, &ni );
        if ( ni.id == NCKEY_ESC ) {
          return {};
        } else if ( ni.id == NCKEY_ENTER ) {
          break;
        }
        selector.offer_input( &ni );
      }

      selector.get_selected( selected.data(), selected.size() );
      std::vector<int> selIndices;
      for ( int i = 0; i < selected.size(); i++ ) {
        if ( selected[i])
          selIndices.push_back(i);
      }

      return selIndices;
    }

    void renderNow () {
      if ( _nc ) _nc->render();
    }

    uint32_t waitForKeys ( std::vector<uint32_t> keys = {} ) {
      ncinput ni;
      while ( true ) {
        _nc->get( true, &ni );
        if ( keys.empty() || std::find( keys.begin(), keys.end(), ni.id ) != keys.end() )
          return ni.id;
      }
    }

  private:
    OutputView() : _topOuterPlane( nullptr ), _topPlaneLeft( nullptr ), _topPlaneRight( nullptr ), _progbar( nullptr, &zypp_release_progbar ) {}

    std::unique_ptr<ncpp::NotCurses> _nc;
    std::unique_ptr<ncpp::Plane> _stdplane;
    std::unique_ptr<ncpp::Plane> _topOuterPlane;
    std::unique_ptr<ncpp::Plane> _topPlaneLeft;
    std::unique_ptr<ncpp::Plane> _topPlaneRight;
    std::unique_ptr<ncpp::Plane> _progBarTextPlane;
    std::unique_ptr<struct ncprogbar, void(*)(struct ncprogbar*)> _progbar;
};


#endif // OUTPUT_H
