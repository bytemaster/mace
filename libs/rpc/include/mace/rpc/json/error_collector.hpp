#ifndef _MACE_RPC_JSON_ERROR_COLLECTOR_HPP_
#define _MACE_RPC_JSON_ERROR_COLLECTOR_HPP_


namespace mace { namespace rpc { namespace json {

  namespace errors {
    enum error_type {
      unknown_error       = 0x0001,  // Other errors not specified below
      warning             = 0x0002,  // Other warnigns not specified below
      sytnax_error        = 0x0004,  // fatal syntax errors unclosed brace
      sytnax_warning      = 0x0008,  // recoverable syntax error (missing, missing ':', unquoted string)
      missing_comma       = 0x0010,  // if the error was related to json syntax and not semantic
      string_to_int       = 0x0020,  // any time lexical cast from string to int is required
      double_to_int       = 0x0040,  // any time a double is received for an int
      int_overflow        = 0x0080,  // any time int value is greater than underlying type
      signed_to_unsigned  = 0x0100,  // any time a negative value is read for an unsigned field
      int_to_bool         = 0x0200,  // any time an int is read for a bool 
      string_to_bool      = 0x0400,  // any time a string is read for a bool
      bad_array_index     = 0x0800,  // atempt to read a vector field beyond end of sequence
      unexpected_key      = 0x1000,  // fields in object
      missing_key         = 0x2000,  // check required fields
      type_mismatch       = 0x4000,  // expected a fundamental, got object, expected object, got array, etc.
      type_conversion     = 0x8000,  // also set any time a conversion occurs
      all                 = 0xffff,
      none                = 0x0000
    };
  } // namespace errors

  /**
   *  Stores information about errors that occurred durring the parse.
   *
   *  By default extra fields are 'ignored' as warning
   *  Loss of presision errors are warning.
   *  String to Int conversion warnings
   *  Double to Int
   *  Int to bool
   *
   */
  struct parse_error {
    parse_error( int32_t ec, std::string msg, char* s = 0, char* e = 0 )
    :message(std::move(msg)),type(ec),start(s),end(e){}

    parse_error( parse_error&& m )
    :message(std::move(m.message)),type(m.type),start(m.start),end(m.end){}

    std::string message;
    int32_t     type;
    char*       start;
    char*       end;
  };

  /**
   *  Collects errors and manages how they are responded to.
   */
  class error_collector : public boost::exception {
    public:
      error_collector( error_collector&& e )
      :m_errors(std::move(e.m_errors)){ 
        memcpy((char*)m_eclass,(char*)e.m_eclass, sizeof(m_eclass) );
      }
      /*
      error_collector( const error_collector&& e )
      :m_errors(e.m_errors){
        memcpy((char*)m_eclass,(char*)e.m_eclass, sizeof(m_eclass) );
      }
      */
      ~error_collector() throw() {
        try {
          m_errors.clear();
        }catch(...){}
      }

      enum error_defaults {
         default_report  = json::errors::all,
         default_recover = json::errors::all,
         default_throw   = json::errors::none,
         default_ignore  = ~(default_report|default_recover|default_throw)
      };

      error_collector(){
        m_eclass[report_error_t]  = default_report;
        m_eclass[recover_error_t] = default_recover;
        m_eclass[throw_error_t]   = default_throw;
        m_eclass[ignore_error_t]  = default_ignore;
      }

      inline bool report( int32_t e )const {
        return m_eclass[report_error_t] & e;
      }
      inline bool recover( int32_t e )const {
        return m_eclass[recover_error_t] & e;
      }
      inline bool ignore( int32_t e )const {
        return m_eclass[ignore_error_t] & e;
      }

      void report_error( int32_t e ) {
        m_eclass[report_error_t] |= e;
        m_eclass[ignore_error_t] &= ~e;
      }
      void recover_error( int32_t e ) {
        m_eclass[recover_error_t] |= e;
        m_eclass[ignore_error_t] &= ~e;
      }
      void throw_error( int32_t e ) {
        m_eclass[throw_error_t]  |= e;
        m_eclass[ignore_error_t] &= ~e;
      }
      void ignore_error( int32_t e ) {
        m_eclass[ignore_error_t]  |= e;
        m_eclass[report_error_t]  &= ~m_eclass[ignore_error_t];
        m_eclass[recover_error_t] &= ~m_eclass[ignore_error_t];
        m_eclass[throw_error_t]   &= ~m_eclass[ignore_error_t];
      }

      void post_error( int32_t ec, std::string msg, char* s = 0, char* e = 0 ) {
        m_errors.push_back( parse_error( ec, std::move(msg), s, e ) ); 
        if( ec & m_eclass[throw_error_t] ) {
          throw std::move(*this);
        }
      }
      const std::vector<parse_error>& get_errors()const {
        return m_errors;
      }

    private:
      enum error_class {
        ignore_error_t,
        report_error_t,
        recover_error_t,
        throw_error_t,
        num_error_classes
      };
      uint32_t m_eclass[num_error_classes];
      std::vector<parse_error>   m_errors;
  };


} } }

#endif // _MACE_RPC_JSON_ERROR_COLLECTOR_HPP_
