import { ComponentFixture, TestBed } from '@angular/core/testing';

import { ListSwreleaseComponent } from './list-swrelease.component';

describe('ListSwreleaseComponent', () => {
  let component: ListSwreleaseComponent;
  let fixture: ComponentFixture<ListSwreleaseComponent>;

  beforeEach(() => {
    TestBed.configureTestingModule({
      declarations: [ListSwreleaseComponent]
    });
    fixture = TestBed.createComponent(ListSwreleaseComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
