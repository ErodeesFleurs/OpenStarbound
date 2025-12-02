//! Command line option parser compatible with C++ Star::OptionParser
//!
//! Provides simple argument parsing with switches, parameters, and positional arguments.

use std::collections::{HashMap, HashSet};
use std::io::Write;

/// Requirement mode for parameters and arguments
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum RequirementMode {
    /// The parameter/argument is optional
    Optional,
    /// The parameter/argument is required
    Required,
    /// The parameter/argument can appear multiple times
    Multiple,
}

/// Parsed command line options
#[derive(Debug, Clone, Default)]
pub struct Options {
    /// Set of switches that were present
    pub switches: HashSet<String>,
    /// Map of parameter names to their values (multiple values possible)
    pub parameters: HashMap<String, Vec<String>>,
    /// List of positional arguments
    pub arguments: Vec<String>,
}

impl Options {
    /// Check if a switch was set
    pub fn has_switch(&self, flag: &str) -> bool {
        self.switches.contains(flag)
    }
    
    /// Get the first value for a parameter
    pub fn get_parameter(&self, flag: &str) -> Option<&str> {
        self.parameters.get(flag).and_then(|v| v.first()).map(|s| s.as_str())
    }
    
    /// Get all values for a parameter
    pub fn get_parameters(&self, flag: &str) -> Option<&Vec<String>> {
        self.parameters.get(flag)
    }
    
    /// Get a positional argument by index
    pub fn get_argument(&self, index: usize) -> Option<&str> {
        self.arguments.get(index).map(|s| s.as_str())
    }
}

/// A switch option (flag without value)
#[derive(Debug, Clone)]
struct Switch {
    flag: String,
    description: String,
}

/// A parameter option (flag with value)
#[derive(Debug, Clone)]
struct Parameter {
    flag: String,
    argument: String,
    requirement_mode: RequirementMode,
    description: String,
}

/// A positional argument
#[derive(Debug, Clone)]
struct Argument {
    name: String,
    requirement_mode: RequirementMode,
    description: String,
}

/// Command line option parser
/// 
/// Compatible with C++ Star::OptionParser. Supports switches, parameters,
/// and positional arguments.
#[derive(Debug, Clone, Default)]
pub struct OptionParser {
    command_name: String,
    summary: String,
    additional_help: String,
    switches: Vec<Switch>,
    parameters: Vec<Parameter>,
    arguments: Vec<Argument>,
}

impl OptionParser {
    /// Create a new option parser
    pub fn new() -> Self {
        Self::default()
    }
    
    /// Set the command name for help text
    pub fn set_command_name(&mut self, name: impl Into<String>) {
        self.command_name = name.into();
    }
    
    /// Set the summary for help text
    pub fn set_summary(&mut self, summary: impl Into<String>) {
        self.summary = summary.into();
    }
    
    /// Set additional help text
    pub fn set_additional_help(&mut self, help: impl Into<String>) {
        self.additional_help = help.into();
    }
    
    /// Add a switch (flag without value)
    pub fn add_switch(&mut self, flag: impl Into<String>, description: impl Into<String>) {
        self.switches.push(Switch {
            flag: flag.into(),
            description: description.into(),
        });
    }
    
    /// Add a parameter (flag with value)
    pub fn add_parameter(
        &mut self,
        flag: impl Into<String>,
        argument_name: impl Into<String>,
        mode: RequirementMode,
        description: impl Into<String>,
    ) {
        self.parameters.push(Parameter {
            flag: flag.into(),
            argument: argument_name.into(),
            requirement_mode: mode,
            description: description.into(),
        });
    }
    
    /// Add a positional argument
    pub fn add_argument(
        &mut self,
        name: impl Into<String>,
        mode: RequirementMode,
        description: impl Into<String>,
    ) {
        self.arguments.push(Argument {
            name: name.into(),
            requirement_mode: mode,
            description: description.into(),
        });
    }
    
    /// Parse command line arguments
    /// 
    /// Returns the parsed options and a list of errors encountered.
    pub fn parse_options(&self, args: &[String]) -> (Options, Vec<String>) {
        let mut options = Options::default();
        let mut errors = Vec::new();
        let mut i = 0;
        
        while i < args.len() {
            let arg = &args[i];
            
            if arg.starts_with('-') {
                let flag = &arg[1..];
                
                // Check if it's a switch
                if self.switches.iter().any(|s| s.flag == flag) {
                    options.switches.insert(flag.to_string());
                    i += 1;
                    continue;
                }
                
                // Check if it's a parameter
                if let Some(param) = self.parameters.iter().find(|p| p.flag == flag) {
                    if i + 1 < args.len() {
                        let value = args[i + 1].clone();
                        options.parameters
                            .entry(flag.to_string())
                            .or_insert_with(Vec::new)
                            .push(value);
                        i += 2;
                    } else {
                        errors.push(format!("Parameter -{} requires a value", flag));
                        i += 1;
                    }
                    continue;
                }
                
                // Unknown flag
                errors.push(format!("Unknown option: {}", arg));
                i += 1;
            } else {
                // Positional argument
                options.arguments.push(arg.clone());
                i += 1;
            }
        }
        
        // Check required parameters
        for param in &self.parameters {
            if param.requirement_mode == RequirementMode::Required {
                if !options.parameters.contains_key(&param.flag) {
                    errors.push(format!("Required parameter -{} not provided", param.flag));
                }
            }
        }
        
        // Check required arguments
        let mut required_count = 0;
        for arg in &self.arguments {
            if arg.requirement_mode == RequirementMode::Required {
                required_count += 1;
            }
        }
        
        if options.arguments.len() < required_count {
            errors.push(format!(
                "Expected at least {} positional argument(s), got {}",
                required_count,
                options.arguments.len()
            ));
        }
        
        (options, errors)
    }
    
    /// Print help text to a writer
    pub fn print_help<W: Write>(&self, w: &mut W) -> std::io::Result<()> {
        if !self.command_name.is_empty() {
            write!(w, "Usage: {}", self.command_name)?;
            
            // Add switches hint
            if !self.switches.is_empty() {
                write!(w, " [options]")?;
            }
            
            // Add arguments
            for arg in &self.arguments {
                match arg.requirement_mode {
                    RequirementMode::Required => write!(w, " <{}>", arg.name)?,
                    RequirementMode::Optional => write!(w, " [{}]", arg.name)?,
                    RequirementMode::Multiple => write!(w, " [{}...]", arg.name)?,
                }
            }
            
            writeln!(w)?;
        }
        
        if !self.summary.is_empty() {
            writeln!(w)?;
            writeln!(w, "{}", self.summary)?;
        }
        
        // Print switches
        if !self.switches.is_empty() {
            writeln!(w)?;
            writeln!(w, "Options:")?;
            
            for switch in &self.switches {
                if switch.description.is_empty() {
                    writeln!(w, "  -{}", switch.flag)?;
                } else {
                    writeln!(w, "  -{:<20} {}", switch.flag, switch.description)?;
                }
            }
        }
        
        // Print parameters
        if !self.parameters.is_empty() {
            if self.switches.is_empty() {
                writeln!(w)?;
                writeln!(w, "Options:")?;
            }
            
            for param in &self.parameters {
                let flag_arg = format!("-{} <{}>", param.flag, param.argument);
                if param.description.is_empty() {
                    writeln!(w, "  {}", flag_arg)?;
                } else {
                    writeln!(w, "  {:<20} {}", flag_arg, param.description)?;
                }
            }
        }
        
        // Print arguments
        if !self.arguments.is_empty() {
            writeln!(w)?;
            writeln!(w, "Arguments:")?;
            
            for arg in &self.arguments {
                let name = match arg.requirement_mode {
                    RequirementMode::Required => format!("<{}>", arg.name),
                    RequirementMode::Optional => format!("[{}]", arg.name),
                    RequirementMode::Multiple => format!("[{}...]", arg.name),
                };
                
                if arg.description.is_empty() {
                    writeln!(w, "  {}", name)?;
                } else {
                    writeln!(w, "  {:<20} {}", name, arg.description)?;
                }
            }
        }
        
        if !self.additional_help.is_empty() {
            writeln!(w)?;
            writeln!(w, "{}", self.additional_help)?;
        }
        
        Ok(())
    }
    
    /// Get help text as a string
    pub fn help_string(&self) -> String {
        let mut buffer = Vec::new();
        self.print_help(&mut buffer).unwrap();
        String::from_utf8(buffer).unwrap()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_simple_switch() {
        let mut parser = OptionParser::new();
        parser.add_switch("v", "Verbose output");
        parser.add_switch("q", "Quiet mode");
        
        let args: Vec<String> = vec!["-v".to_string()];
        let (options, errors) = parser.parse_options(&args);
        
        assert!(errors.is_empty());
        assert!(options.has_switch("v"));
        assert!(!options.has_switch("q"));
    }
    
    #[test]
    fn test_parameter() {
        let mut parser = OptionParser::new();
        parser.add_parameter("o", "file", RequirementMode::Required, "Output file");
        
        let args: Vec<String> = vec!["-o".to_string(), "output.txt".to_string()];
        let (options, errors) = parser.parse_options(&args);
        
        assert!(errors.is_empty());
        assert_eq!(options.get_parameter("o"), Some("output.txt"));
    }
    
    #[test]
    fn test_required_parameter_missing() {
        let mut parser = OptionParser::new();
        parser.add_parameter("o", "file", RequirementMode::Required, "Output file");
        
        let args: Vec<String> = vec![];
        let (_, errors) = parser.parse_options(&args);
        
        assert!(!errors.is_empty());
        assert!(errors[0].contains("-o"));
    }
    
    #[test]
    fn test_positional_arguments() {
        let mut parser = OptionParser::new();
        parser.add_argument("input", RequirementMode::Required, "Input file");
        parser.add_argument("output", RequirementMode::Optional, "Output file");
        
        let args: Vec<String> = vec!["input.txt".to_string(), "output.txt".to_string()];
        let (options, errors) = parser.parse_options(&args);
        
        assert!(errors.is_empty());
        assert_eq!(options.get_argument(0), Some("input.txt"));
        assert_eq!(options.get_argument(1), Some("output.txt"));
    }
    
    #[test]
    fn test_required_argument_missing() {
        let mut parser = OptionParser::new();
        parser.add_argument("input", RequirementMode::Required, "Input file");
        
        let args: Vec<String> = vec![];
        let (_, errors) = parser.parse_options(&args);
        
        assert!(!errors.is_empty());
    }
    
    #[test]
    fn test_multiple_parameters() {
        let mut parser = OptionParser::new();
        parser.add_parameter("i", "file", RequirementMode::Multiple, "Input file");
        
        let args: Vec<String> = vec![
            "-i".to_string(), "a.txt".to_string(),
            "-i".to_string(), "b.txt".to_string(),
        ];
        let (options, errors) = parser.parse_options(&args);
        
        assert!(errors.is_empty());
        let params = options.get_parameters("i").unwrap();
        assert_eq!(params.len(), 2);
        assert_eq!(params[0], "a.txt");
        assert_eq!(params[1], "b.txt");
    }
    
    #[test]
    fn test_unknown_option() {
        let parser = OptionParser::new();
        
        let args: Vec<String> = vec!["-x".to_string()];
        let (_, errors) = parser.parse_options(&args);
        
        assert!(!errors.is_empty());
        assert!(errors[0].contains("Unknown"));
    }
    
    #[test]
    fn test_help_string() {
        let mut parser = OptionParser::new();
        parser.set_command_name("myapp");
        parser.set_summary("A sample application");
        parser.add_switch("v", "Enable verbose mode");
        parser.add_parameter("o", "file", RequirementMode::Required, "Output file");
        parser.add_argument("input", RequirementMode::Required, "Input file");
        
        let help = parser.help_string();
        
        assert!(help.contains("myapp"));
        assert!(help.contains("verbose"));
        assert!(help.contains("Output file"));
        assert!(help.contains("Input file"));
    }
    
    #[test]
    fn test_mixed_args() {
        let mut parser = OptionParser::new();
        parser.add_switch("v", "Verbose");
        parser.add_parameter("n", "count", RequirementMode::Optional, "Count");
        parser.add_argument("file", RequirementMode::Required, "File");
        
        let args: Vec<String> = vec![
            "-v".to_string(),
            "-n".to_string(), "5".to_string(),
            "myfile.txt".to_string(),
        ];
        let (options, errors) = parser.parse_options(&args);
        
        assert!(errors.is_empty());
        assert!(options.has_switch("v"));
        assert_eq!(options.get_parameter("n"), Some("5"));
        assert_eq!(options.get_argument(0), Some("myfile.txt"));
    }
}
