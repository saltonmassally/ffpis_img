/*
 *
 * Stan Janet
 * December 4, 1990
 *
 */

#define XMGetSubImageDataDepth(_src,_x,_y,_srcw,_srch,_dst,_dstw,_dsth) \
{ \
int _i, _dst_index, _src_index; \
\
if (_x < 0) \
	_x = 0; \
else { \
	if (_x > _srcw - _dstw) \
		_x = (_srcw - _dstw); \
} \
\
if (_y < 0) \
	_y = 0; \
else { \
	if (_y > _srch - _dsth) \
		_y = _srch - _dsth; \
} \
\
_src_index = _y * _srcw + _x; \
_dst_index = 0; \
_i = _dsth; \
while (_i-- > 0) { \
	(void) memcpy(&(_dst[_dst_index]), &(_src[_src_index]), (int)(_dstw)); \
	_dst_index += _dstw; \
	_src_index += _srcw; \
} \
}
