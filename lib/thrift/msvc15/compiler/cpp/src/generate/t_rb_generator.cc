/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 * Contains some contributions under the Thrift Software License.
 * Please see doc/old-thrift-license.txt in the Thrift distribution for
 * details.
 */

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sstream>

#include "t_oop_generator.h"
#include "platform.h"
#include "version.h"

using namespace std;


/**
 * Ruby code generator.
 *
 */
class t_rb_generator : public t_oop_generator {
 public:
  t_rb_generator(
      t_program* program,
      const std::map<std::string, std::string>& parsed_options,
      const std::string& option_string)
    : t_oop_generator(program)
  {
    (void) parsed_options;
    (void) option_string;
    out_dir_base_ = "gen-rb";
  }

  /**
   * Init and close methods
   */

  void init_generator();
  void close_generator();

  /**
   * Program-level generation functions
   */

  void generate_typedef     (t_typedef*  ttypedef);
  void generate_enum        (t_enum*     tenum);
  void generate_const       (t_const*    tconst);
  void generate_struct      (t_struct*   tstruct);
  void generate_union        (t_struct*   tunion);
  void generate_xception    (t_struct*   txception);
  void generate_service     (t_service*  tservice);

  std::string render_const_value(t_type* type, t_const_value* value);

  /**
   * Struct generation code
   */

  void generate_rb_struct(std::ofstream& out, t_struct* tstruct, bool is_exception);
  void generate_rb_struct_required_validator(std::ofstream& out, t_struct* tstruct);
  void generate_rb_union(std::ofstream& out, t_struct* tstruct, bool is_exception);
  void generate_rb_union_validator(std::ofstream& out, t_struct* tstruct);
  void generate_rb_function_helpers(t_function* tfunction);
  void generate_rb_simple_constructor(std::ofstream& out, t_struct* tstruct);
  void generate_rb_simple_exception_constructor(std::ofstream& out, t_struct* tstruct);
  void generate_field_constants (std::ofstream& out, t_struct* tstruct);
  void generate_field_constructors (std::ofstream& out, t_struct* tstruct);
  void generate_field_defns (std::ofstream& out, t_struct* tstruct);
  void generate_field_data  (std::ofstream& out, t_type* field_type, const std::string& field_name, t_const_value* field_value, bool optional);

  /**
   * Service-level generation functions
   */

  void generate_service_helpers   (t_service*  tservice);
  void generate_service_interface (t_service* tservice);
  void generate_service_client    (t_service* tservice);
  void generate_service_server    (t_service* tservice);
  void generate_process_function  (t_service* tservice, t_function* tfunction);

  /**
   * Serialization constructs
   */

  void generate_deserialize_field        (std::ofstream &out,
                                          t_field*    tfield,
                                          std::string prefix="",
                                          bool inclass=false);

  void generate_deserialize_struct       (std::ofstream &out,
                                          t_struct*   tstruct,
                                          std::string prefix="");

  void generate_deserialize_container    (std::ofstream &out,
                                          t_type*     ttype,
                                          std::string prefix="");

  void generate_deserialize_set_element  (std::ofstream &out,
                                          t_set*      tset,
                                          std::string prefix="");

  void generate_deserialize_map_element  (std::ofstream &out,
                                          t_map*      tmap,
                                          std::string prefix="");

  void generate_deserialize_list_element (std::ofstream &out,
                                          t_list*     tlist,
                                          std::string prefix="");

  void generate_serialize_field          (std::ofstream &out,
                                          t_field*    tfield,
                                          std::string prefix="");

  void generate_serialize_struct         (std::ofstream &out,
                                          t_struct*   tstruct,
                                          std::string prefix="");

  void generate_serialize_container      (std::ofstream &out,
                                          t_type*     ttype,
                                          std::string prefix="");

  void generate_serialize_map_element    (std::ofstream &out,
                                          t_map*      tmap,
                                          std::string kiter,
                                          std::string viter);

  void generate_serialize_set_element    (std::ofstream &out,
                                          t_set*      tmap,
                                          std::string iter);

  void generate_serialize_list_element   (std::ofstream &out,
                                          t_list*     tlist,
                                          std::string iter);

  void generate_rdoc                     (std::ofstream& out, 
                                          t_doc* tdoc);

  /**
   * Helper rendering functions
   */

  std::string rb_autogen_comment();
  std::string render_includes();
  std::string declare_field(t_field* tfield);
  std::string type_name(t_type* ttype);
  std::string full_type_name(t_type* ttype);
  std::string function_signature(t_function* tfunction, std::string prefix="");
  std::string argument_list(t_struct* tstruct);
  std::string type_to_enum(t_type* ttype);



  std::vector<std::string> ruby_modules(t_program* p) {
    std::string ns = p->get_namespace("rb");
    std::vector<std::string> modules;
    if (ns.empty()) {
      return modules;
    }

    std::string::iterator pos = ns.begin();
    while (true) {
      std::string::iterator delim = std::find(pos, ns.end(), '.');
      modules.push_back(capitalize(std::string(pos, delim)));
      pos = delim;
      if (pos == ns.end()) {
        break;
      }
      ++pos;
    }

    return modules;
  }

  void begin_namespace(std::ofstream&, std::vector<std::string>);
  void end_namespace(std::ofstream&, std::vector<std::string>);

 private:

  /**
   * File streams
   */

  std::ofstream f_types_;
  std::ofstream f_consts_;
  std::ofstream f_service_;

};


/**
 * Prepares for file generation by opening up the necessary file output
 * streams.
 *
 * @param tprogram The program to generate
 */
void t_rb_generator::init_generator() {
  // Make output directory
  MKDIR(get_out_dir().c_str());

  // Make output file
  string f_types_name = get_out_dir()+underscore(program_name_)+"_types.rb";
  f_types_.open(f_types_name.c_str());

  string f_consts_name = get_out_dir()+underscore(program_name_)+"_constants.rb";
  f_consts_.open(f_consts_name.c_str());

  // Print header
  f_types_ <<
    rb_autogen_comment() << endl <<
    render_includes() << endl;
    begin_namespace(f_types_, ruby_modules(program_));

  f_consts_ <<
    rb_autogen_comment() << endl <<
    "require '" << underscore(program_name_) << "_types'" << endl <<
    endl;
    begin_namespace(f_consts_, ruby_modules(program_));

}

/**
 * Renders all the imports necessary for including another Thrift program
 */
string t_rb_generator::render_includes() {
  const vector<t_program*>& includes = program_->get_includes();
  string result = "";
  for (size_t i = 0; i < includes.size(); ++i) {
    result += "require '" + underscore(includes[i]->get_name()) + "_types'\n";
  }
  if (includes.size() > 0) {
    result += "\n";
  }
  return result;
}

/**
 * Autogen'd comment
 */
string t_rb_generator::rb_autogen_comment() {
  return
    std::string("#\n") +
    "# Autogenerated by Thrift Compiler (" + THRIFT_VERSION + ")\n" +
    "#\n" +
    "# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING\n" +
    "#\n";
}

/**
 * Closes the type files
 */
void t_rb_generator::close_generator() {
  // Close types file
  end_namespace(f_types_, ruby_modules(program_));
  end_namespace(f_consts_, ruby_modules(program_));
  f_types_.close();
  f_consts_.close();
}

/**
 * Generates a typedef. This is not done in Ruby, types are all implicit.
 *
 * @param ttypedef The type definition
 */
void t_rb_generator::generate_typedef(t_typedef* ttypedef) {
  (void) ttypedef;
}

/**
 * Generates code for an enumerated type. Done using a class to scope
 * the values.
 *
 * @param tenum The enumeration
 */
void t_rb_generator::generate_enum(t_enum* tenum) {
  indent(f_types_) <<
    "module " << capitalize(tenum->get_name()) << endl;
  indent_up();

  vector<t_enum_value*> constants = tenum->get_constants();
  vector<t_enum_value*>::iterator c_iter;
  for (c_iter = constants.begin(); c_iter != constants.end(); ++c_iter) {
    int value = (*c_iter)->get_value();

    // Ruby class constants have to be capitalized... omg i am so on the fence
    // about languages strictly enforcing capitalization why can't we just all
    // agree and play nice.
    string name = capitalize((*c_iter)->get_name());

    generate_rdoc(f_types_, *c_iter);
    indent(f_types_) << name << " = " << value << endl;
  }
  
  // Create a hash mapping values back to their names (as strings) since ruby has no native enum type
  indent(f_types_) << "VALUE_MAP = {";
  bool first = true;
  for(c_iter = constants.begin(); c_iter != constants.end(); ++c_iter) {
    // Populate the hash
    int value = (*c_iter)->get_value();
    first ? first = false : static_cast<bool>(f_types_ << ", ");
    f_types_ << value << " => \"" << capitalize((*c_iter)->get_name()) << "\"";
  }
  f_types_ << "}" << endl;
  
  // Create a set with valid values for this enum
  indent(f_types_) << "VALID_VALUES = Set.new([";
  first = true;
  for (c_iter = constants.begin(); c_iter != constants.end(); ++c_iter) {
    // Populate the set
    first ? first = false : static_cast<bool>(f_types_ << ", ");
    f_types_ << capitalize((*c_iter)->get_name());
  }
  f_types_ << "]).freeze" << endl;

  indent_down();
  indent(f_types_) <<
    "end" << endl << endl;
}

/**
 * Generate a constant value
 */
void t_rb_generator::generate_const(t_const* tconst) {
  t_type* type = tconst->get_type();
  string name = tconst->get_name();
  t_const_value* value = tconst->get_value();

  name[0] = toupper(name[0]);

  indent(f_consts_) << name << " = " << render_const_value(type, value);
  f_consts_ << endl << endl;
}

/**
 * Prints the value of a constant with the given type. Note that type checking
 * is NOT performed in this function as it is always run beforehand using the
 * validate_types method in main.cc
 */
string t_rb_generator::render_const_value(t_type* type, t_const_value* value) {
  type = get_true_type(type);
  std::ostringstream out;
  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
    switch (tbase) {
    case t_base_type::TYPE_STRING:
      out << "%q\"" << get_escaped_string(value) << '"';
      break;
    case t_base_type::TYPE_BOOL:
      out << (value->get_integer() > 0 ? "true" : "false");
      break;
    case t_base_type::TYPE_BYTE:
    case t_base_type::TYPE_I16:
    case t_base_type::TYPE_I32:
    case t_base_type::TYPE_I64:
      out << value->get_integer();
      break;
    case t_base_type::TYPE_DOUBLE:
      if (value->get_type() == t_const_value::CV_INTEGER) {
        out << value->get_integer();
      } else {
        out << value->get_double();
      }
      break;
    default:
      throw "compiler error: no const of base type " + t_base_type::t_base_name(tbase);
    }
  } else if (type->is_enum()) {
    indent(out) << value->get_integer();
  } else if (type->is_struct() || type->is_xception()) {
    out << full_type_name(type) << ".new({" << endl;
    indent_up();
    const vector<t_field*>& fields = ((t_struct*)type)->get_members();
    vector<t_field*>::const_iterator f_iter;
    const map<t_const_value*, t_const_value*>& val = value->get_map();
    map<t_const_value*, t_const_value*>::const_iterator v_iter;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      t_type* field_type = NULL;
      for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
        if ((*f_iter)->get_name() == v_iter->first->get_string()) {
          field_type = (*f_iter)->get_type();
        }
      }
      if (field_type == NULL) {
        throw "type error: " + type->get_name() + " has no field " + v_iter->first->get_string();
      }
      out << indent();
      out << render_const_value(g_type_string, v_iter->first);
      out << " => ";
      out << render_const_value(field_type, v_iter->second);
      out << "," << endl;
    }
    indent_down();
    indent(out) << "})";
  } else if (type->is_map()) {
    t_type* ktype = ((t_map*)type)->get_key_type();
    t_type* vtype = ((t_map*)type)->get_val_type();
    out << "{" << endl;
    indent_up();
    const map<t_const_value*, t_const_value*>& val = value->get_map();
    map<t_const_value*, t_const_value*>::const_iterator v_iter;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      out << indent();
      out << render_const_value(ktype, v_iter->first);
      out << " => ";
      out << render_const_value(vtype, v_iter->second);
      out << "," << endl;
    }
    indent_down();
    indent(out) << "}";
  } else if (type->is_list() || type->is_set()) {
    t_type* etype;
    if (type->is_list()) {
      etype = ((t_list*)type)->get_elem_type();
    } else {
      etype = ((t_set*)type)->get_elem_type();
    }
    if (type->is_set()) {
      out << "Set.new([" << endl;
    } else {
      out << "[" << endl;
    }
    indent_up();
    const vector<t_const_value*>& val = value->get_list();
    vector<t_const_value*>::const_iterator v_iter;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      out << indent();
      out << render_const_value(etype, *v_iter);
      out << "," << endl;
    }
    indent_down();
    if (type->is_set()) {
      indent(out) << "])";
    } else {
      indent(out) << "]";
    }
  } else {
    throw "CANNOT GENERATE CONSTANT FOR TYPE: " + type->get_name();
  }
  return out.str();
}

/**
 * Generates a ruby struct
 */
void t_rb_generator::generate_struct(t_struct* tstruct) {
  if (tstruct->is_union()) {
    generate_rb_union(f_types_, tstruct, false);
  } else {
    generate_rb_struct(f_types_, tstruct, false);
  }
}

/**
 * Generates a struct definition for a thrift exception. Basically the same
 * as a struct but extends the Exception class.
 *
 * @param txception The struct definition
 */
void t_rb_generator::generate_xception(t_struct* txception) {
  generate_rb_struct(f_types_, txception, true);
}

/**
 * Generates a ruby struct
 */
void t_rb_generator::generate_rb_struct(std::ofstream& out, t_struct* tstruct, bool is_exception = false) {
  generate_rdoc(out, tstruct);
  indent(out) << "class " << type_name(tstruct);
  if (is_exception) {
    out << " < ::Thrift::Exception";
  }
  out << endl;

  indent_up();
  indent(out) << "include ::Thrift::Struct, ::Thrift::Struct_Union" << endl;

  if (is_exception) {
    generate_rb_simple_exception_constructor(out, tstruct);
  }

  generate_field_constants(out, tstruct);
  generate_field_defns(out, tstruct);
  generate_rb_struct_required_validator(out, tstruct);

  indent(out) << "::Thrift::Struct.generate_accessors self" << endl;

  indent_down();
  indent(out) << "end" << endl << endl;
}


/**
 * Generates a ruby union
 */
void t_rb_generator::generate_rb_union(std::ofstream& out, t_struct* tstruct, bool is_exception = false) {
  (void) is_exception;
  generate_rdoc(out, tstruct);
  indent(out) << "class " << type_name(tstruct) << " < ::Thrift::Union" << endl;

  indent_up();
  indent(out) << "include ::Thrift::Struct_Union" << endl;

  generate_field_constructors(out, tstruct);

  generate_field_constants(out, tstruct);
  generate_field_defns(out, tstruct);
  generate_rb_union_validator(out, tstruct);

  indent(out) << "::Thrift::Union.generate_accessors self" << endl;

  indent_down();
  indent(out) << "end" << endl << endl;
}

void t_rb_generator::generate_field_constructors(std::ofstream& out, t_struct* tstruct) {

  indent(out) << "class << self" << endl;
  indent_up();

  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;

  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if (f_iter != fields.begin()) {
      out << endl;
    }
    std::string field_name = (*f_iter)->get_name();

    indent(out) << "def " << field_name << "(val)" << endl;
    indent(out) << "  " << tstruct->get_name() << ".new(:" << field_name << ", val)" << endl;
    indent(out) << "end" << endl;
  }
  
  indent_down();
  indent(out) << "end" << endl;

  out << endl;
}

void t_rb_generator::generate_rb_simple_exception_constructor(std::ofstream& out, t_struct* tstruct) {
  const vector<t_field*>& members = tstruct->get_members();

  if (members.size() == 1) {
    vector<t_field*>::const_iterator m_iter = members.begin();

    if ((*m_iter)->get_type()->is_string()) {
      string name = (*m_iter)->get_name();

      indent(out) << "def initialize(message=nil)" << endl;
      indent_up();
      indent(out) << "super()" << endl;
      indent(out) << "self." << name << " = message" << endl;
      indent_down();
      indent(out) << "end" << endl << endl;

      if (name != "message") {
        indent(out) << "def message; " << name << " end" << endl << endl;
      }
    }
  }
}

void t_rb_generator::generate_field_constants(std::ofstream& out, t_struct* tstruct) {
  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;

  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    std::string field_name = (*f_iter)->get_name();
    std::string cap_field_name = upcase_string(field_name);
    
    indent(out) << cap_field_name << " = " << (*f_iter)->get_key() << endl;
  }
  out << endl;
}

void t_rb_generator::generate_field_defns(std::ofstream& out, t_struct* tstruct) {
  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;

  indent(out) << "FIELDS = {" << endl;
  indent_up();
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if (f_iter != fields.begin()) {
      out << "," << endl;
    }

    // generate the field docstrings within the FIELDS constant. no real better place...
    generate_rdoc(out, *f_iter);

    indent(out) <<
      upcase_string((*f_iter)->get_name()) << " => ";

    generate_field_data(out, (*f_iter)->get_type(), (*f_iter)->get_name(), (*f_iter)->get_value(), 
      (*f_iter)->get_req() == t_field::T_OPTIONAL);
  }
  indent_down();
  out << endl;
  indent(out) << "}" << endl << endl;
  
  indent(out) << "def struct_fields; FIELDS; end" << endl << endl;
  
}

void t_rb_generator::generate_field_data(std::ofstream& out, t_type* field_type,
    const std::string& field_name = "", t_const_value* field_value = NULL, bool optional = false) {
  field_type = get_true_type(field_type);

  // Begin this field's defn
  out << "{:type => " << type_to_enum(field_type);

  if (!field_name.empty()) {
    out << ", :name => '" << field_name << "'";
  }

  if (field_value != NULL) {
    out << ", :default => " << render_const_value(field_type, field_value);
  }

  if (!field_type->is_base_type()) {
    if (field_type->is_struct() || field_type->is_xception()) {
      out << ", :class => " << full_type_name((t_struct*)field_type);
    } else if (field_type->is_list()) {
      out << ", :element => ";
      generate_field_data(out, ((t_list*)field_type)->get_elem_type());
    } else if (field_type->is_map()) {
      out << ", :key => ";
      generate_field_data(out, ((t_map*)field_type)->get_key_type());
      out << ", :value => ";
      generate_field_data(out, ((t_map*)field_type)->get_val_type());
    } else if (field_type->is_set()) {
      out << ", :element => ";
      generate_field_data(out, ((t_set*)field_type)->get_elem_type());
    }
  } else {
    if (((t_base_type*)field_type)->is_binary()) {
      out << ", :binary => true";
    }
  }
  
  if(optional) {
    out << ", :optional => true";
  }

  if (field_type->is_enum()) {
    out << ", :enum_class => " << full_type_name(field_type);
  }

  // End of this field's defn
  out << "}";
}

void t_rb_generator::begin_namespace(std::ofstream& out, vector<std::string> modules) {
  for (vector<std::string>::iterator m_iter = modules.begin(); m_iter != modules.end(); ++m_iter) {
    indent(out) << "module " << *m_iter << endl;
    indent_up();
  }
}

void t_rb_generator::end_namespace(std::ofstream& out, vector<std::string> modules) {
  for (vector<std::string>::reverse_iterator m_iter = modules.rbegin(); m_iter != modules.rend(); ++m_iter) {
    indent_down();
    indent(out) << "end" << endl;
  }
}


/**
 * Generates a thrift service.
 *
 * @param tservice The service definition
 */
void t_rb_generator::generate_service(t_service* tservice) {
  string f_service_name = get_out_dir()+underscore(service_name_)+".rb";
  f_service_.open(f_service_name.c_str());

  f_service_ <<
    rb_autogen_comment() << endl <<
    "require 'thrift'" << endl;

  if (tservice->get_extends() != NULL) {
    f_service_ <<
      "require '" << underscore(tservice->get_extends()->get_name()) << "'" << endl;
  }

  f_service_ <<
    "require '" << underscore(program_name_) << "_types'" << endl <<
    endl;

  begin_namespace(f_service_, ruby_modules(tservice->get_program()));

  indent(f_service_) << "module " << capitalize(tservice->get_name()) << endl;
  indent_up();

  // Generate the three main parts of the service (well, two for now in PHP)
  generate_service_client(tservice);
  generate_service_server(tservice);
  generate_service_helpers(tservice);

  indent_down();
  indent(f_service_) << "end" << endl <<
    endl;

  end_namespace(f_service_, ruby_modules(tservice->get_program()));

  // Close service file
  f_service_.close();
}

/**
 * Generates helper functions for a service.
 *
 * @param tservice The service to generate a header definition for
 */
void t_rb_generator::generate_service_helpers(t_service* tservice) {
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator f_iter;

  indent(f_service_) <<
    "# HELPER FUNCTIONS AND STRUCTURES" << endl << endl;

  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    t_struct* ts = (*f_iter)->get_arglist();
    generate_rb_struct(f_service_, ts);
    generate_rb_function_helpers(*f_iter);
  }
}

/**
 * Generates a struct and helpers for a function.
 *
 * @param tfunction The function
 */
void t_rb_generator::generate_rb_function_helpers(t_function* tfunction) {
  t_struct result(program_, tfunction->get_name() + "_result");
  t_field success(tfunction->get_returntype(), "success", 0);
  if (!tfunction->get_returntype()->is_void()) {
    result.append(&success);
  }

  t_struct* xs = tfunction->get_xceptions();
  const vector<t_field*>& fields = xs->get_members();
  vector<t_field*>::const_iterator f_iter;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    result.append(*f_iter);
  }
  generate_rb_struct(f_service_, &result);
}

/**
 * Generates a service client definition.
 *
 * @param tservice The service to generate a server for.
 */
void t_rb_generator::generate_service_client(t_service* tservice) {
  string extends = "";
  string extends_client = "";
  if (tservice->get_extends() != NULL) {
    extends = full_type_name(tservice->get_extends());
    extends_client = " < " + extends + "::Client ";
  }

  indent(f_service_) <<
    "class Client" << extends_client << endl;
  indent_up();

  indent(f_service_) <<
    "include ::Thrift::Client" << endl << endl;

  // Generate client method implementations
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::const_iterator f_iter;
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    t_struct* arg_struct = (*f_iter)->get_arglist();
    const vector<t_field*>& fields = arg_struct->get_members();
    vector<t_field*>::const_iterator fld_iter;
    string funname = (*f_iter)->get_name();

    // Open function
    indent(f_service_) <<
      "def " << function_signature(*f_iter) << endl;
    indent_up();
      indent(f_service_) <<
        "send_" << funname << "(";

      bool first = true;
      for (fld_iter = fields.begin(); fld_iter != fields.end(); ++fld_iter) {
        if (first) {
          first = false;
        } else {
          f_service_ << ", ";
        }
        f_service_ << (*fld_iter)->get_name();
      }
      f_service_ << ")" << endl;

      if (!(*f_iter)->is_oneway()) {
        f_service_ << indent();
        if (!(*f_iter)->get_returntype()->is_void()) {
          f_service_ << "return ";
        }
        f_service_ <<
          "recv_" << funname << "()" << endl;
      }
    indent_down();
    indent(f_service_) << "end" << endl;
    f_service_ << endl;

    indent(f_service_) <<
      "def send_" << function_signature(*f_iter) << endl;
    indent_up();

      std::string argsname = capitalize((*f_iter)->get_name() + "_args");

      indent(f_service_) << "send_message('" << funname << "', " << argsname;

      for (fld_iter = fields.begin(); fld_iter != fields.end(); ++fld_iter) {
        f_service_ << ", :" << (*fld_iter)->get_name() << " => " << (*fld_iter)->get_name();
      }

      f_service_ << ")" << endl;

    indent_down();
    indent(f_service_) << "end" << endl;

    if (!(*f_iter)->is_oneway()) {
      std::string resultname = capitalize((*f_iter)->get_name() + "_result");
      t_struct noargs(program_);

      t_function recv_function((*f_iter)->get_returntype(),
                               string("recv_") + (*f_iter)->get_name(),
                               &noargs);
      // Open function
      f_service_ <<
        endl <<
        indent() << "def " << function_signature(&recv_function) << endl;
      indent_up();

      // TODO(mcslee): Validate message reply here, seq ids etc.

      f_service_ <<
        indent() << "result = receive_message(" << resultname << ")" << endl;

      // Careful, only return _result if not a void function
      if (!(*f_iter)->get_returntype()->is_void()) {
        f_service_ <<
          indent() << "return result.success unless result.success.nil?" << endl;
      }

      t_struct* xs = (*f_iter)->get_xceptions();
      const std::vector<t_field*>& xceptions = xs->get_members();
      vector<t_field*>::const_iterator x_iter;
      for (x_iter = xceptions.begin(); x_iter != xceptions.end(); ++x_iter) {
        indent(f_service_) <<
          "raise result." << (*x_iter)->get_name() <<
            " unless result." << (*x_iter)->get_name() << ".nil?" << endl;
      }

      // Careful, only return _result if not a void function
      if ((*f_iter)->get_returntype()->is_void()) {
        indent(f_service_) <<
          "return" << endl;
      } else {
        f_service_ <<
          indent() << "raise ::Thrift::ApplicationException.new(::Thrift::ApplicationException::MISSING_RESULT, '" << (*f_iter)->get_name() << " failed: unknown result')" << endl;
      }

      // Close function
      indent_down();
      indent(f_service_) << "end" << endl << endl;
    }
  }

  indent_down();
  indent(f_service_) << "end" << endl << endl;
}

/**
 * Generates a service server definition.
 *
 * @param tservice The service to generate a server for.
 */
void t_rb_generator::generate_service_server(t_service* tservice) {
  // Generate the dispatch methods
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator f_iter;

  string extends = "";
  string extends_processor = "";
  if (tservice->get_extends() != NULL) {
    extends = full_type_name(tservice->get_extends());
    extends_processor = " < " + extends + "::Processor ";
  }

  // Generate the header portion
  indent(f_service_) <<
    "class Processor" << extends_processor << endl;
  indent_up();

  f_service_ <<
    indent() << "include ::Thrift::Processor" << endl <<
    endl;

  // Generate the process subfunctions
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    generate_process_function(tservice, *f_iter);
  }

  indent_down();
  indent(f_service_) << "end" << endl << endl;
}

/**
 * Generates a process function definition.
 *
 * @param tfunction The function to write a dispatcher for
 */
void t_rb_generator::generate_process_function(t_service* tservice,
                                               t_function* tfunction) {
  (void) tservice;
  // Open function
  indent(f_service_) <<
    "def process_" << tfunction->get_name() <<
    "(seqid, iprot, oprot)" << endl;
  indent_up();

  string argsname = capitalize(tfunction->get_name()) + "_args";
  string resultname = capitalize(tfunction->get_name()) + "_result";

  f_service_ <<
    indent() << "args = read_args(iprot, " << argsname << ")" << endl;

  t_struct* xs = tfunction->get_xceptions();
  const std::vector<t_field*>& xceptions = xs->get_members();
  vector<t_field*>::const_iterator x_iter;

  // Declare result for non oneway function
  if (!tfunction->is_oneway()) {
    f_service_ <<
      indent() << "result = " << resultname << ".new()" << endl;
  }

  // Try block for a function with exceptions
  if (xceptions.size() > 0) {
    f_service_ <<
      indent() << "begin" << endl;
    indent_up();
  }

  // Generate the function call
  t_struct* arg_struct = tfunction->get_arglist();
  const std::vector<t_field*>& fields = arg_struct->get_members();
  vector<t_field*>::const_iterator f_iter;

  f_service_ << indent();
  if (!tfunction->is_oneway() && !tfunction->get_returntype()->is_void()) {
    f_service_ << "result.success = ";
  }
  f_service_ <<
    "@handler." << tfunction->get_name() << "(";
  bool first = true;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if (first) {
      first = false;
    } else {
      f_service_ << ", ";
    }
    f_service_ << "args." << (*f_iter)->get_name();
  }
  f_service_ << ")" << endl;

  if (!tfunction->is_oneway() && xceptions.size() > 0) {
    indent_down();
    for (x_iter = xceptions.begin(); x_iter != xceptions.end(); ++x_iter) {
      f_service_ <<
        indent() << "rescue " << full_type_name((*x_iter)->get_type()) << " => " << (*x_iter)->get_name() << endl;
      if (!tfunction->is_oneway()) {
        indent_up();
        f_service_ <<
          indent() << "result." << (*x_iter)->get_name() << " = " << (*x_iter)->get_name() << endl;
        indent_down();
      }
    }
    indent(f_service_) << "end" << endl;
  }

  // Shortcut out here for oneway functions
  if (tfunction->is_oneway()) {
    f_service_ <<
      indent() << "return" << endl;
    indent_down();
    indent(f_service_) << "end" << endl << endl;
    return;
  }

  f_service_ <<
    indent() << "write_result(result, oprot, '" << tfunction->get_name() << "', seqid)" << endl;

  // Close function
  indent_down();
  indent(f_service_) << "end" << endl << endl;
}

/**
 * Renders a function signature of the form 'type name(args)'
 *
 * @param tfunction Function definition
 * @return String of rendered function definition
 */
string t_rb_generator::function_signature(t_function* tfunction,
                                           string prefix) {
  // TODO(mcslee): Nitpicky, no ',' if argument_list is empty
  return
    prefix + tfunction->get_name() +
    "(" +  argument_list(tfunction->get_arglist()) + ")";
}

/**
 * Renders a field list
 */
string t_rb_generator::argument_list(t_struct* tstruct) {
  string result = "";

  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;
  bool first = true;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if (first) {
      first = false;
    } else {
      result += ", ";
    }
    result += (*f_iter)->get_name();
  }
  return result;
}

string t_rb_generator::type_name(t_type* ttype) {
  string prefix = "";

  string name = ttype->get_name();
  if (ttype->is_struct() || ttype->is_xception() || ttype->is_enum()) {
    name = capitalize(ttype->get_name());
  }

  return prefix + name;
}

string t_rb_generator::full_type_name(t_type* ttype) {
  string prefix = "";
  vector<std::string> modules = ruby_modules(ttype->get_program());
  for (vector<std::string>::iterator m_iter = modules.begin();
       m_iter != modules.end(); ++m_iter) {
    prefix += *m_iter + "::";
  }
  return prefix + type_name(ttype);
}

/**
 * Converts the parse type to a Ruby tyoe
 */
string t_rb_generator::type_to_enum(t_type* type) {
  type = get_true_type(type);

  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
    switch (tbase) {
    case t_base_type::TYPE_VOID:
      throw "NO T_VOID CONSTRUCT";
    case t_base_type::TYPE_STRING:
      return "::Thrift::Types::STRING";
    case t_base_type::TYPE_BOOL:
      return "::Thrift::Types::BOOL";
    case t_base_type::TYPE_BYTE:
      return "::Thrift::Types::BYTE";
    case t_base_type::TYPE_I16:
      return "::Thrift::Types::I16";
    case t_base_type::TYPE_I32:
      return "::Thrift::Types::I32";
    case t_base_type::TYPE_I64:
      return "::Thrift::Types::I64";
    case t_base_type::TYPE_DOUBLE:
      return "::Thrift::Types::DOUBLE";
    }
  } else if (type->is_enum()) {
    return "::Thrift::Types::I32";
  } else if (type->is_struct() || type->is_xception()) {
    return "::Thrift::Types::STRUCT";
  } else if (type->is_map()) {
    return "::Thrift::Types::MAP";
  } else if (type->is_set()) {
    return "::Thrift::Types::SET";
  } else if (type->is_list()) {
    return "::Thrift::Types::LIST";
  }

  throw "INVALID TYPE IN type_to_enum: " + type->get_name();
}


void t_rb_generator::generate_rdoc(std::ofstream& out, t_doc* tdoc) {
  if (tdoc->has_doc()) {
    generate_docstring_comment(out,
      "", "# ", tdoc->get_doc(), "");
  }
}

void t_rb_generator::generate_rb_struct_required_validator(std::ofstream& out, 
                                                           t_struct* tstruct) {
  indent(out) << "def validate" << endl;
  indent_up();

  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;

  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    t_field* field = (*f_iter);
    if (field->get_req() == t_field::T_REQUIRED) {
      indent(out) << "raise ::Thrift::ProtocolException.new(::Thrift::ProtocolException::UNKNOWN, 'Required field " << field->get_name() << " is unset!')";
      if (field->get_type()->is_bool()) {
        out << " if @" << field->get_name() << ".nil?";
      } else {
        out << " unless @" << field->get_name();
      }
      out << endl;
    }
  }

  // if field is an enum, check that its value is valid
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    t_field* field = (*f_iter);

    if (field->get_type()->is_enum()){      
      indent(out) << "unless @" << field->get_name() << ".nil? || " << full_type_name(field->get_type()) << "::VALID_VALUES.include?(@" << field->get_name() << ")" << endl;
      indent_up();
      indent(out) << "raise ::Thrift::ProtocolException.new(::Thrift::ProtocolException::UNKNOWN, 'Invalid value of field " << field->get_name() << "!')" << endl;  
      indent_down();
      indent(out) << "end" << endl;
    }
  }

  indent_down();
  indent(out) << "end" << endl << endl;
}

void t_rb_generator::generate_rb_union_validator(std::ofstream& out, 
                                                 t_struct* tstruct) {
  indent(out) << "def validate" << endl;
  indent_up();

  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;

  indent(out) << "raise(StandardError, 'Union fields are not set.') if get_set_field.nil? || get_value.nil?" << endl;

  // if field is an enum, check that its value is valid
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    const t_field* field = (*f_iter);

    if (field->get_type()->is_enum()){      
      indent(out) << "if get_set_field == :" << field->get_name() << endl;
      indent(out) << "  raise ::Thrift::ProtocolException.new(::Thrift::ProtocolException::UNKNOWN, 'Invalid value of field " << field->get_name() << "!') unless " << full_type_name(field->get_type()) << "::VALID_VALUES.include?(get_value)" << endl;  
      indent(out) << "end" << endl;
    }
  }

  indent_down();
  indent(out) << "end" << endl << endl;
}

THRIFT_REGISTER_GENERATOR(rb, "Ruby", "")

