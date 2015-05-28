//most of this is shamelessly stolen from the amazing collection of IITC plugins. I stand on the shoulders of giants.
//for those portions the following license applies
/* 
Copyright © 2013 Stefan Breunig
Permission to use, copy, modify, and/or distribute this software for
any purpose with or without fee is hereby granted, provided that the
above copyright notice and this permission notice appear in all
copies.
THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
*/

var L = {};

var zeroPad = function(number,pad) {
 number = number.toString();
 var zeros = pad - number.length;
 return Array(zeros>0?zeros+1:0).join("0") + number;
};

var LatLngToXYZ = function(latLng) {
  var d2r = Math.PI/180.0;

  var phi = latLng.lat*d2r;
  var theta = latLng.lng*d2r;

  var cosphi = Math.cos(phi);

  return [Math.cos(theta)*cosphi, Math.sin(theta)*cosphi, Math.sin(phi)];
};

var XYZToLatLng = function(xyz) {
  var r2d = 180.0/Math.PI;

  var lat = Math.atan2(xyz[2], Math.sqrt(xyz[0]*xyz[0]+xyz[1]*xyz[1]));
  var lng = Math.atan2(xyz[1], xyz[0]);

  return L.latLng(lat*r2d, lng*r2d);
}; 

var largestAbsComponent = function(xyz) {
  var temp = [Math.abs(xyz[0]), Math.abs(xyz[1]), Math.abs(xyz[2])];

  if (temp[0] > temp[1]) {
    if (temp[0] > temp[2]) {
      return 0;
    } else {
      return 2;
    }
  } else {
    if (temp[1] > temp[2]) {
      return 1;
    } else {
      return 2;
    }
  }

};

var faceXYZToUV = function(face,xyz) {
  var u,v;

  switch (face) {
    case 0: u =  xyz[1]/xyz[0]; v =  xyz[2]/xyz[0]; break;
    case 1: u = -xyz[0]/xyz[1]; v =  xyz[2]/xyz[1]; break;
    case 2: u = -xyz[0]/xyz[2]; v = -xyz[1]/xyz[2]; break;
    case 3: u =  xyz[2]/xyz[0]; v =  xyz[1]/xyz[0]; break;
    case 4: u =  xyz[2]/xyz[1]; v = -xyz[0]/xyz[1]; break;
    case 5: u = -xyz[1]/xyz[2]; v = -xyz[0]/xyz[2]; break;
    default: throw {error: 'Invalid face'}; 
  }

  return [u,v];
};




var XYZToFaceUV = function(xyz) {
  var face = largestAbsComponent(xyz);

  if (xyz[face] < 0) {
    face += 3;
  }

  var uv = faceXYZToUV (face,xyz);

  return [face, uv];
};

var FaceUVToXYZ = function(face,uv) {
  var u = uv[0];
  var v = uv[1];

  switch (face) {
    case 0: return [ 1, u, v];
    case 1: return [-u, 1, v];
    case 2: return [-u,-v, 1];
    case 3: return [-1,-v,-u];
    case 4: return [ v,-1,-u];
    case 5: return [ v, u,-1];
    default: throw {error: 'Invalid face'};
  }
};


var STToUV = function(st) {
  var singleSTtoUV = function(st) {
    if (st >= 0.5) {
      return (1/3.0) * (4*st*st - 1);
    } else {
      return (1/3.0) * (1 - (4*(1-st)*(1-st)));
    }
  };

  return [singleSTtoUV(st[0]), singleSTtoUV(st[1])];
};



var UVToST = function(uv) {
  var singleUVtoST = function(uv) {
    if (uv >= 0) {
      return 0.5 * Math.sqrt (1 + 3*uv);
    } else {
      return 1 - 0.5 * Math.sqrt (1 - 3*uv);
    }
  };

  return [singleUVtoST(uv[0]), singleUVtoST(uv[1])];
};


var STToIJ = function(st,order) {
  var maxSize = (1<<order);

  var singleSTtoIJ = function(st) {
    var ij = Math.floor(st * maxSize);
    return Math.max(0, Math.min(maxSize-1, ij));
  };

  return [singleSTtoIJ(st[0]), singleSTtoIJ(st[1])];
};


var IJToST = function(ij,order,offsets) {
  var maxSize = (1<<order);

  return [
    (ij[0]+offsets[0])/maxSize,
    (ij[1]+offsets[1])/maxSize
  ];
};

// hilbert space-filling curve
// based on http://blog.notdot.net/2009/11/Damn-Cool-Algorithms-Spatial-indexing-with-Quadtrees-and-Hilbert-Curves
// note: rather then calculating the final integer hilbert position, we just return the list of quads
// this ensures no precision issues whth large orders (S3 cell IDs use up to 30), and is more
// convenient for pulling out the individual bits as needed later
var pointToHilbertQuadList = function(x,y,order) {
  var hilbertMap = {
    'a': [ [0,'d'], [1,'a'], [3,'b'], [2,'a'] ],
    'b': [ [2,'b'], [1,'b'], [3,'a'], [0,'c'] ],
    'c': [ [2,'c'], [3,'d'], [1,'c'], [0,'b'] ],
    'd': [ [0,'a'], [3,'c'], [1,'d'], [2,'d'] ]  
  };

  var currentSquare='a';
  var positions = [];

  for (var i=order-1; i>=0; i--) {

    var mask = 1<<i;

    var quad_x = x&mask ? 1 : 0;
    var quad_y = y&mask ? 1 : 0;

    var t = hilbertMap[currentSquare][quad_x*2+quad_y];

    positions.push(t[0]);

    currentSquare = t[1];
  }

  return positions;
};


var S2 = {};

// S2Cell class

S2.S2Cell = function(){};

//static method to construct
S2.S2Cell.FromLatLng = function(latLng,level) {

  var xyz = LatLngToXYZ(latLng);

  var faceuv = XYZToFaceUV(xyz);
  var st = UVToST(faceuv[1]);

  var ij = STToIJ(st,level);

  return S2.S2Cell.FromFaceIJ (faceuv[0], ij, level);

  
};

S2.S2Cell.FromFaceIJ = function(face,ij,level) {
  var cell = new S2.S2Cell();
  cell.face = face;
  cell.ij = ij;
  cell.level = level;

  return cell;
};


S2.S2Cell.prototype.toString = function() {
  return 'F'+this.face+'ij['+this.ij[0]+','+this.ij[1]+']@'+this.level;
};

S2.S2Cell.prototype.getLatLng = function() {
  var st = IJToST(this.ij,this.level, [0.5,0.5]);
  var uv = STToUV(st);
  var xyz = FaceUVToXYZ(this.face, uv);

  return XYZToLatLng(xyz);  
};

S2.S2Cell.prototype.getCornerLatLngs = function() {
  var result = [];
  var offsets = [
    [ 0.0, 0.0 ],
    [ 0.0, 1.0 ],
    [ 1.0, 1.0 ],
    [ 1.0, 0.0 ]
  ];

  for (var i=0; i<4; i++) {
    var st = IJToST(this.ij, this.level, offsets[i]);
    var uv = STToUV(st);
    var xyz = FaceUVToXYZ(this.face, uv);

    result.push ( XYZToLatLng(xyz) );
  }
  return result;
};


S2.S2Cell.prototype.getFaceAndQuads = function() {
  var quads = pointToHilbertQuadList(this.ij[0], this.ij[1], this.level);

  return [this.face,quads];
};

S2.S2Cell.prototype.getNeighbors = function() {

  var fromFaceIJWrap = function(face,ij,level) {
    var maxSize = (1<<level);
    if (ij[0]>=0 && ij[1]>=0 && ij[0]<maxSize && ij[1]<maxSize) {
      // no wrapping out of bounds
      return S2.S2Cell.FromFaceIJ(face,ij,level);
    } else {
      // the new i,j are out of range.
      // with the assumption that they're only a little past the borders we can just take the points as
      // just beyond the cube face, project to XYZ, then re-create FaceUV from the XYZ vector

      var st = IJToST(ij,level,[0.5,0.5]);
      var uv = STToUV(st);
      var xyz = FaceUVToXYZ(face,uv);
      var faceuv = XYZToFaceUV(xyz);
      face = faceuv[0];
      uv = faceuv[1];
      st = UVToST(uv);
      ij = STToIJ(st,level);
      return S2.S2Cell.FromFaceIJ (face, ij, level);
    }
  };

  var face = this.face;
  var i = this.ij[0];
  var j = this.ij[1];
  var level = this.level;


  return [
    fromFaceIJWrap(face, [i-1,j], level),
    fromFaceIJWrap(face, [i,j-1], level),
    fromFaceIJWrap(face, [i+1,j], level),
    fromFaceIJWrap(face, [i,j+1], level)
  ];

};
var regionName = function(cell) {
  var face2name = [ 'AF', 'AS', 'NR', 'PA', 'AM', 'ST' ];
  var codeWord = [
    'ALPHA',
    'BRAVO',
    'CHARLIE',
    'DELTA',
    'ECHO',
    'FOXTROT',
    'GOLF',
    'HOTEL',
    'JULIET',
    'KILO',
    'LIMA',
    'MIKE',
    'NOVEMBER',
    'PAPA',
    'ROMEO',
    'SIERRA'
  ];


  // ingress does some odd things with the naming. for some faces, the i and j coords are flipped when converting
  // (and not only the names - but the full quad coords too!). easiest fix is to create a temporary cell with the coords
  // swapped
  if (cell.face == 1 || cell.face == 3 || cell.face == 5) {
    cell = S2.S2Cell.FromFaceIJ ( cell.face, [cell.ij[1], cell.ij[0]], cell.level );
  }

  // first component of the name is the face
  var name = face2name[cell.face];

  if (cell.level >= 4) {
    // next two components are from the most signifitant four bits of the cell I/J
    var regionI = cell.ij[0] >> (cell.level-4);
    var regionJ = cell.ij[1] >> (cell.level-4);

    name += zeroPad(regionI+1,2)+'-'+codeWord[regionJ];
  }

  if (cell.level >= 6) {
    // the final component is based on the hibbert curve for the relevant cell
    var facequads = cell.getFaceAndQuads();
    var number = facequads[1][4]*4+facequads[1][5];

    name += '-'+zeroPad(number,2);
  }


  return name;
};

var locationOptions = {  enableHighAccuracy: false, "timeout": 10000, "maximumAge": 600000 };
var locationWatcher;
//var nextsend=0;
var d = new Date();

function locationError(err) { 
  console.log('Failed to acquire position');
    Pebble.sendAppMessage( { '0': d.getTimezoneOffset()+2400, '1': "NO-GPS-LOCK" },
    function(e) { console.log('Success');   },
    function(e) { console.log('Failure'); } );
    return true;
}

function locationSuccess(pos)
{
  var coordinates = pos.coords;
  var latlng = {};
  latlng.lat = coordinates.latitude;
  latlng.lng = coordinates.longitude;
  var cell = S2.S2Cell.FromLatLng ( latlng , 6 );

    Pebble.sendAppMessage( { '0': d.getTimezoneOffset()+2400, '1': regionName(cell) },
    function(e) { console.log('Success'); },
    function(e) { console.log('Failure'); }
    ); 
    
  return true;
}

function updatePebble(e)
{
  locationWatcher = navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
}
Pebble.addEventListener('ready', updatePebble);
Pebble.addEventListener('appmessage',updatePebble);
Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
  var topics=[];
  Pebble.timelineSubscriptions(
      function (topics) {
          console.log('Subscribed to ' + topics.join(', '));
        },
       function (errorString) {
         console.log('Error getting subscriptions: ' + errorString);

       }
      );
  var current_watch;
  if(Pebble.getActiveWatchInfo) {
      try {
        current_watch = Pebble.getActiveWatchInfo();
      } catch(err) {
        current_watch = {
          platform: "basalt",
        };
      }
    } else {
      current_watch = {
        platform: "aplite",
      };
    }
    console.log('Using watch info: ' + JSON.stringify(current_watch));
  //Pebble.openURL('http://api.mudkips.net/static/ingressconfig.html?topics='+topics.join(',')+"&platform="+current_watch.platform);
  Pebble.openURL('http://x.SetPebble.com/GQR3/' + Pebble.getAccountToken());
Pebble.addEventListener('webviewclosed',
  function(e) {
    if ((typeof(e.response) == 'string') && (e.response.length > 0)) {
    console.log(e.response);
    var configuration = JSON.parse(decodeURIComponent(e.response));
    console.log('Configuration window returned: ', JSON.stringify(configuration));
    if (configuration) {
      if (configuration[1]) { 
        Pebble.timelineSubscribe('shard',function () { console.log('Subscribed: shard'); }, function (errorString) { console.log('Error subscribing to topic: ' + errorString); } );
        } else {
        Pebble.timelineUnsubscribe('shard',function () { console.log('Unsubscribed: shard'); }, function (errorString) { console.log('Error subscribing to topic: ' + errorString); } );
        }
       if (configuration[2]) { 
        Pebble.timelineSubscribe('shardscore',function () { console.log('Subscribed: shardscore'); }, function (errorString) { console.log('Error subscribing to topic: ' + errorString); } );
        } else {
        Pebble.timelineUnsubscribe('shardscore',function () { console.log('Unsubscribed: shardscore'); }, function (errorString) { console.log('Error subscribing to topic: ' + errorString); } );
        }
        if (configuration[3]) { 
        Pebble.timelineSubscribe('anomaly',function () { console.log('Subscribed: anomaly'); }, function (errorString) { console.log('Error subscribing to topic: ' + errorString); } );
        } else {
        Pebble.timelineUnsubscribe('anomaly',function () { console.log('Unsubscribed: anomaly'); }, function (errorString) { console.log('Error subscribing to topic: ' + errorString); } );
        }
        if (configuration[4]) { 
        Pebble.timelineSubscribe('anomalyscore',function () { console.log('Subscribed: anomalyscore'); }, function (errorString) { console.log('Error subscribing to topic: ' + errorString); } );
        } else {
        Pebble.timelineUnsubscribe('anomalyscore',function () { console.log('Unsubscribed: anomalyscore'); }, function (errorString) { console.log('Error subscribing to topic: ' + errorString); } );
        }
    }
    }
  });
});