<?php

class Committers {

  var $committers_cache_file = "/tmp/committers-cache";
  var $committers;

  function Committers() {
    // PHP 4 does not have destructors so we register a destructor like this:
    register_shutdown_function(array(&$this, '_Committers'));

    // Load the cache file from disk
    $this->committers = $this->load_committers_cache();
  }

  function _Committers() {
    $this->prune_committers_cache();
    $this->store_committers_cache();
  }


  function get_committers($hash1, $hash2) {

    if(preg_match("/Unknown/", "$hash1$hash2")) {
      return Array();
    }

    $authors = $this->lookup_committers($hash1, $hash2);

    if(is_null($authors)) {
      // Try to form a list of people who committed during this period
      $cmd = "cd /space/git/CONDOR_SRC.git; git log $hash1..$hash2 2>&1";
      $log = preg_split("/\n/", `$cmd`);

      $authors = Array();
      foreach ($log as $line) {
        if(preg_match("/^\s*Author:\s*(.+)(\s+\<(.+)\>)?$/", $line, $matches)) {
          if(array_key_exists($matches[1], $authors)) {
            $authors[$matches[1]]["count"] += 1;
          }
          else {
            $authors[$matches[1]] = Array();
            $authors[$matches[1]]["count"] = 1;
            $authors[$matches[1]]["email"] = 0;
          }
        }
      }

      // Store this in the cache
      $this->committers["$hash1$hash2"] = $authors;
    }

    return $authors;
  }


  function lookup_committers($hash1, $hash2) {
    if(array_key_exists("$hash1$hash2", $this->committers)) {
      return $this->committers["$hash1$hash2"];
    }
    return NULL;
  }


  function store_committers_cache() {
    $handle = fopen($this->committers_cache_file, "w");
    fwrite($handle, serialize($this->committers));
    fclose($handle);
  }

  // Load the file that holds the committers cache
  function load_committers_cache() {
    if(file_exists($this->committers_cache_file)) {
      $handle = fopen($this->committers_cache_file, "r");
      $contents = fread($handle, filesize($this->committers_cache_file));
      fclose($handle);

      return unserialize($contents);
    }
    return Array();
  }

  // Remove anything that has not been accessed in ?? days
  function prune_committers_cache() {
    // Empty for now until we put in access records
  }

}

?>