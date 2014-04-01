define(["jquery"], function ($) {
  "use strict";

  // Toggle visibility of fact sources
  $(".js-toggle-sources").on("click", function(e) {
    e.preventDefault();

    // Toggle visibility of fact sources wrapper
    $(".sources").slideToggle();
  });
});