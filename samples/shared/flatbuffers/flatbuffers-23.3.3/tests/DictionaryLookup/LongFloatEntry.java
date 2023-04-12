// automatically generated by the FlatBuffers compiler, do not modify

package DictionaryLookup;

import com.google.flatbuffers.BaseVector;
import com.google.flatbuffers.BooleanVector;
import com.google.flatbuffers.ByteVector;
import com.google.flatbuffers.Constants;
import com.google.flatbuffers.DoubleVector;
import com.google.flatbuffers.FlatBufferBuilder;
import com.google.flatbuffers.FloatVector;
import com.google.flatbuffers.IntVector;
import com.google.flatbuffers.LongVector;
import com.google.flatbuffers.ShortVector;
import com.google.flatbuffers.StringVector;
import com.google.flatbuffers.Struct;
import com.google.flatbuffers.Table;
import com.google.flatbuffers.UnionVector;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

@SuppressWarnings("unused")
public final class LongFloatEntry extends Table {
  public static void ValidateVersion() { Constants.FLATBUFFERS_23_3_3(); }
  public static LongFloatEntry getRootAsLongFloatEntry(ByteBuffer _bb) { return getRootAsLongFloatEntry(_bb, new LongFloatEntry()); }
  public static LongFloatEntry getRootAsLongFloatEntry(ByteBuffer _bb, LongFloatEntry obj) { _bb.order(ByteOrder.LITTLE_ENDIAN); return (obj.__assign(_bb.getInt(_bb.position()) + _bb.position(), _bb)); }
  public void __init(int _i, ByteBuffer _bb) { __reset(_i, _bb); }
  public LongFloatEntry __assign(int _i, ByteBuffer _bb) { __init(_i, _bb); return this; }

  public long key() { int o = __offset(4); return o != 0 ? bb.getLong(o + bb_pos) : 0L; }
  public float value() { int o = __offset(6); return o != 0 ? bb.getFloat(o + bb_pos) : 0.0f; }

  public static int createLongFloatEntry(FlatBufferBuilder builder,
      long key,
      float value) {
    builder.startTable(2);
    LongFloatEntry.addKey(builder, key);
    LongFloatEntry.addValue(builder, value);
    return LongFloatEntry.endLongFloatEntry(builder);
  }

  public static void startLongFloatEntry(FlatBufferBuilder builder) { builder.startTable(2); }
  public static void addKey(FlatBufferBuilder builder, long key) { builder.addLong(key); builder.slot(0); }
  public static void addValue(FlatBufferBuilder builder, float value) { builder.addFloat(1, value, 0.0f); }
  public static int endLongFloatEntry(FlatBufferBuilder builder) {
    int o = builder.endTable();
    return o;
  }

  @Override
  protected int keysCompare(Integer o1, Integer o2, ByteBuffer _bb) {
    long val_1 = _bb.getLong(__offset(4, o1, _bb));
    long val_2 = _bb.getLong(__offset(4, o2, _bb));
    return val_1 > val_2 ? 1 : val_1 < val_2 ? -1 : 0;
  }

  public static LongFloatEntry __lookup_by_key(LongFloatEntry obj, int vectorLocation, long key, ByteBuffer bb) {
    int span = bb.getInt(vectorLocation - 4);
    int start = 0;
    while (span != 0) {
      int middle = span / 2;
      int tableOffset = __indirect(vectorLocation + 4 * (start + middle), bb);
      long val = bb.getLong(__offset(4, bb.capacity() - tableOffset, bb));
      int comp = val > key ? 1 : val < key ? -1 : 0;
      if (comp > 0) {
        span = middle;
      } else if (comp < 0) {
        middle++;
        start += middle;
        span -= middle;
      } else {
        return (obj == null ? new LongFloatEntry() : obj).__assign(tableOffset, bb);
      }
    }
    return null;
  }

  public static final class Vector extends BaseVector {
    public Vector __assign(int _vector, int _element_size, ByteBuffer _bb) { __reset(_vector, _element_size, _bb); return this; }

    public LongFloatEntry get(int j) { return get(new LongFloatEntry(), j); }
    public LongFloatEntry get(LongFloatEntry obj, int j) {  return obj.__assign(__indirect(__element(j), bb), bb); }
    public LongFloatEntry getByKey(long key) {  return __lookup_by_key(null, __vector(), key, bb); }
    public LongFloatEntry getByKey(LongFloatEntry obj, long key) {  return __lookup_by_key(obj, __vector(), key, bb); }
  }
}

